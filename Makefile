CPPFLAGS=-g -Wall

test: test_allocator
	./test_allocator

test_allocator: mmap_allocator.h

clean:
	rm test_allocator testfile
