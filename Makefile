CPPFLAGS=-g -Wall
CFLAGS=-g -Wall

# Enable to test with GCC 3.4
# CXX=g++34

PREFIX=/usr

SOURCES=mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h

HEADERS=mmap_access_mode.h mmap_allocator.h mmap_file_pool.h mmap_exception.h
LIBRARIES=libmmap_allocator.so

SRC_INSTALL_TARGET_DIR=/home/johannes/re3

all: test_allocator mmap_file_pool.o libmmap_allocator.so

debug: CPPFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: CFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: clean all

libmmap_allocator.so: mmap_file_pool.o
	g++ -shared -o libmmap_allocator.so mmap_file_pool.o

install_sources: $(SOURCES)
	cp $(SOURCES) $(SRC_INSTALL_TARGET_DIR)

install: all
	install -m 644 $(HEADERS) $(PREFIX)/include
	install -m 755 $(LIBRARIES) $(PREFIX)/lib
	
test: all
	@echo "Running mmap allocator regression test suite."
	./test_allocator

debugtest: debug
	@echo "Running mmap allocator regression test suite with verbose enabled."
	./test_allocator

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o libmmap_allocator.so
	g++ -L. -lmmap_allocator test_allocator.o -o test_allocator

test_mmap_fixed: test_mmap_fixed.c
	gcc $(CFLAGS) test_mmap_fixed.c -o test_mmap_fixed

clean:
	rm -f test_allocator test_mmap_fixed testfile *.o libmmap_allocator.so

mmap_file_pool.o: mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h

test_allocator.o: test_allocator.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h
