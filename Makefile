CPPFLAGS=-g -Wall

all: test mmap_file_pool.o

test: test_allocator test_mmap_fixed
	./test_allocator

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o
	g++ mmap_file_pool.o test_allocator.o -o test_allocator

test_mmap_fixed: test_mmap_fixed.c
	gcc test_mmap_fixed.c -o test_mmap_fixed

clean:
	rm -f test_allocator test_mmap_fixed testfile *.o
