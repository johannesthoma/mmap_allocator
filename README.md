mmap_allocator - A STL allocator that mmaps files
-------------------------------------------------

Introduction
------------

When reading large files (>100MB) into memory, read() calls are usually 
not very space and time efficient, since the whole data is copiied at
least once. Furthermore, when using STL containers (like a vector for
example), data is copiied another time unless you specify the location
of the vector content as parameter to read(). 

It would be nice to tell the vector a filename and have the vector mmap
the file directly. This not only avoids the read() copiing (and the
STL vector copiing) but also allows different processes that read the
same file to see the same physical memory. Fortunately STL foresees
an interface to do exactly this.

Put short, if you need to handle big files that contain unstructured data 
(like doubles or even text files), mmap_allocator is worth a try. It was
written as part of a consulting project I did for a large Austrian bank.

License
-------

This code is (c) Johannes Thoma 2012 and is LGPL. Please write me an email to 
johannes.thoma@gmx.at if you find this useful, or if you find bugs or want 
to extend the library.

How it works
------------

Many STL container classes accept allocators as part of their instantiation
and construction. The default allocator simply does a malloc on allocate and
a free on deallocate. By subclassing the default allocator and replacing the
allocate and deallocate methods with code that memory maps a file and return
a pointer to the mmaped area, we can create STL containers that use memory
mapped regions directly.

Usage
-----

To build this, do a 

        make

followed by a 

        make install

When compiling, include the mmappable_vector.h file and link with 
the -lmmap_allocator library.

Example
-------

Suppose you have a file with 1024 ints. Then:

	mmappable_vector<int> my_vector;

declares a vector that uses mmap_allocator to mmap the file. By calling:

	my_vector.mmap_file("testfile", READ_ONLY, 0, 1024);

the mmappable_vector class will call allocate of the mmap_allocator
class, which in turn will mmap the file and set the content of the 
vector to the mmapped area. The file's content can then be accessed 
the usual way (my_vector[i]). 

Use 

	my_vector.munmap_file();

to drop the mapping (it may remain in an internal cache, however). After
this call all accesses of mmappable_vector elements are invalid, same
goes for the iterators.

Do not forget to:

	using namespace mmap_allocator_namespace;

and include:

	#include "mmappable_vector.h"

but you probably know that yourself ;)

Please see the test_allocator.cpp file for more examples 
(I used Testdriven development for this project and can highly recommend
this development model).

Mode
----

As part of the construction of the mmap_allocator object a mode field can be 
specified. It does the following:

* DEFAULT_STL_ALLOCATOR: Default STL allocator (malloc based). Reason 
	is to have containers that do both and are compatible
* READ_ONLY: Readonly modus. Segfaults when vector content is written to.
* READ_WRITE_PRIVATE: Read/write access, writes are not propagated to disk.
* READ_WRITE_SHARED: Read/write access, writes are propagated to disk 
	(file is modified)

The offset parameter must be page aligned (PAGE_SIZE, usually 4K or 8K), 
else mmap will return an error.

Mmap file pool
--------------

Beginning with 0.3, mmap allocator uses a pool of mmaped files. If you
map the same file with the same access mode again it gets the same
(virtual and physical) memory assigned as it was mapped previously.
We needed this because our program (that uses mmap_allocator) mmaps
a file in many small chunks (10K junks) which eventually lead to a
Out of filedescriptors error (when mapping a 100Meg file in 10K junks).

Following flags can be passed (by |'ing them together) to the mmap_file
method:

* MAP_WHOLE_FILE: Set this flag when you know that you need
the whole (or most parts of the) file later and only want to 
request a part of it now.

* ALLOW_REMAP: Set this flag if you are allocating a vector
from a mmapped file and you know that you do not need previous
mappings any more. Normally this is used when the file size 
changes (in particular when it grows). Be aware, however that
this could invalidate all allocations for that file that have
been made before.

* BYPASS_FILE_POOL: Set this flag if you want a per-vector
private mapping. This is useful in conjunction with READ_WRITE_PRIVATE
mappings.

Mmappable vector class
----------------------

Beginning with version 0.4, there is a special std::vector subclass 
for use with mmapped vectors. This was necessary because there is
no standard way to set the size of an STL vector without initializing
the content (which is not wanted since the content is coming from the
mmapped file). From now on, please use the mmapped_vector class for
using mmap_allocator.

The old method by using the mmap_allocators constructor to set the
parameters for file mapping is still supported, however deprecated.

READ_WRITE caveats
------------------

Unlike reading from a file, a mmappable_vector that is mapped via
the mmap file pool contains all changes already made to the content
before. Suppose if you have:

    mmappable_vector<int> p;
    if (method == MMAP) {
	p.mmap_file("testfile", READ_WRITE_PRIVATE, 0, filesize("testfile")); 
    } else {
        readFromFileIntoVector(p, "testfile", READ_WRITE_PRIVATE, 0, filesize("testfile")
    }

and then do something like:

    for (it = p.begin();it != p.end();it++) {
       *it += 1;
    }

This will do not what you would expect at first glance when being called
twice. When the vector maps the file for the second time, it will map it
from the file pool and hence already have the values increased by one.

To avoid this, use the bypass_file_pool flag which will cause the file 
being mmapped another time with different virtual memory mappings.

Exceptions
----------

There is a custom exception class mmap_allocator_exception which is
used at various places, for example when the file to be mapped is 
not found. Use the e.message() method to find out what happened.

Conversion function templates
-----------------------------

To convert between a STL vector and a mmappable vector, use the
to_std_vector(mappable_vector) and to_mmappable_vector(std_vector)
function templates. However, try to avoid this because this will
copy the whole vector contents.

Verbosity
---------

To debug the allocator, there are two ways: 

* call set_verbosity() with a positive value at runtime.
* define MMAP_ALLOCATOR_DEBUG as a positive value at compile time.

The latter is done by make test.

Version history
---------------

* 0.1.0, first release.
* 0.2.0, some interface changes.
* 0.3.0, mmaped file pool.
* 0.3.1, do not remap files when area fits in already mapped file.
* 0.3.2, never use MAP_FIXED.
* 0.3.3, bugfix in computing pointers.
* 0.4.0, mmapped_vector class.
* 0.5.0, cleaner interface, illegal offset bugfix.
* 0.5.1, bypass_file_pool flag.
* 0.5.2, changed bool parameters to flags argument.
* 0.5.3, set_verbosity() to toggle debug output.
* 0.6.0, to_stl_vector function template.
* 0.6.1, flags bugfix.
* 0.7.0, moved mmappable_vector to separate header.
* 0.8.0, to_mmappable_vector conversion function.
* 0.8.1, exception now knows what() method.
* 0.9.0, more standard conformant exception handling.
* 0.9.1, fixed a permission bug in MMAP_READWRITE_PRIVATE.

Author
------

This was written by Johannes Thoma <johannes.thoma@gmx.at>
Thanks to Piotr Nycz from stackoverflow for contributing 
the constructor function template in mmappable_vector.h
