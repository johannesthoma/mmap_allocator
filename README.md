mmap_allocator - A STL allocator that mmaps files
-------------------------------------------------

Introduction
------------

When reading large files (>10GB) into memory, read() calls are usually 
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
written as part of a consulting project I did for a big Austrian bank.

License
-------

This code (c) Johannes Thoma 2012 and is LGPL. Please write me an email to 
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

To use this, simply copy the mmap_allocator.h file into your project
and include it. No compilation of the library itself is required (the
Makefile will compile and run the regression test file).

Example
-------

Suppose you have a file with 1024 ints. Then:

	vector<int, mmap_allocator<int> > my_vector = 
		vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile"));

declares a vector that uses mmap_allocator to mmap the file. By calling:

	my_vector.reserve(1024);

the STL vector class will call allocate which in turn will mmap the file and 
set the content of the vector to the mmapped area. The file's content can
then be accessed the usual way (my_vector[i]).

Do not forget to:
	using namespace std;
	using namespace mmap_allocator_namespace;

and include:

	#include "mmap_allocator.h"
	#include <vector>

but you probably know that yourself ;)

Please see the test_allocator.cpp file for more examples (\footnote{
I used Testdriven development for this project and can highly recommend
this}).

Mode
----

As part of the construction of the mmap_allocator object a mode field can be 
specified. It does the following:

DEFAULT_STL_ALLOCATOR: Default STL allocator (malloc based). Reason 
	is to have containers that do both and are compatible
READ_ONLY: Readonly modus. Segfaults when vector content is written to.
READ_WRITE_PRIVATE: Read/write access, writes are not propagated to disk.
READ_WRITE_SHARED: Read/write access, writes are propagated to disk 
	(file is modified)

The offset parameter must be page aligned (PAGE_SIZE, usually 4K or 8K), 
else mmap will return an error.

Version
-------

I would say this is 0.1, first release.

Author
------

This was written by Johannes Thoma <johannes.thoma@gmx.at>
