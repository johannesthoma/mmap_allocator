CPPFLAGS=-g -Wall

all: test mmap_file_pool.o

test: test_allocator
	./test_allocator

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o
	g++ mmap_file_pool.o test_allocator.o -o test_allocator

clean:
	rm -f test_allocator testfile *.o
