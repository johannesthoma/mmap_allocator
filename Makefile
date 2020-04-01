CPPFLAGS=-g -Wall -fPIC
CFLAGS=-g -Wall -fPIC

# Enable to test with GCC 3.4
# CXX=g++34

PREFIX=/usr

SOURCES=mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h

HEADERS=mmap_access_mode.h mmap_allocator.h mmap_file_pool.h mmap_exception.h mmappable_vector.h
LIBRARIES=libmmap_allocator.so libmmap_allocator.a

SRC_INSTALL_TARGET_DIR=/home/johannes/re3

all: test_allocator mmap_file_pool.o $(LIBRARIES)

debug: CPPFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: CFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: clean all

libmmap_allocator.so: mmap_file_pool.o
	g++ -shared -o libmmap_allocator.so mmap_file_pool.o

libmmap_allocator.a: mmap_file_pool.o
	ar r libmmap_allocator.a mmap_file_pool.o

install_sources: $(SOURCES)
	cp $(SOURCES) $(SRC_INSTALL_TARGET_DIR)

install: all
	install -m 644 $(HEADERS) $(PREFIX)/include
	install -m 755 $(LIBRARIES) $(PREFIX)/lib
	
test: all
	install -Dm 644 -t $(DESTDIR)$(PREFIX)/include $(HEADERS)
	install -Dm 755 -t $(DESTDIR)$(PREFIX)/lib $(LIBRARIES)
	@echo "Running mmap allocator regression test suite."
	bash -c 'export LD_LIBRARY_PATH=. ; ./test_allocator'

debugtest: debug
	@echo "Running mmap allocator regression test suite with verbose enabled."
	bash -c 'export LD_LIBRARY_PATH=. ; ./test_allocator'

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o $(LIBRARIES)
	g++ test_allocator.o -L. -lmmap_allocator -o test_allocator

test_mmap_fixed: test_mmap_fixed.c
	gcc $(CFLAGS) test_mmap_fixed.c -o test_mmap_fixed

clean:
	rm -f test_allocator test_mmap_fixed testfile testfile2 *.o $(LIBRARIES)

mmap_file_pool.o: mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h

test_allocator.o: test_allocator.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h
