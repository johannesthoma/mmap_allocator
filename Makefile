CPPFLAGS=-g -Wall
CFLAGS=-g -Wall

all: test mmap_file_pool.o

test: test_allocator test_mmap_fixed
	@echo "Running mmap allocator regression test suite."
	./test_allocator
	@echo "Running MAP_FIXED test, if this Segfaults then your glibc/kernel is probably broken"
	./test_mmap_fixed

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o
	g++ mmap_file_pool.o test_allocator.o -o test_allocator

test_mmap_fixed: test_mmap_fixed.c
	gcc $(CFLAGS) test_mmap_fixed.c -o test_mmap_fixed

clean:
	rm -f test_allocator test_mmap_fixed testfile *.o

mmap_file_pool.o: mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h

test_allocator.o: mmap_file_pool.h mmap_allocator.h mmap_access_mode.h
