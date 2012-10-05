CPPFLAGS=-g -Wall
CFLAGS=-g -Wall

TARGET_DIR=/home/johannes/re3

all: test mmap_file_pool.o

debug: CPPFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: CFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: clean all

install: mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h
	cp mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h $(TARGET_DIR)

test: test_allocator
	@echo "Running mmap allocator regression test suite."
	./test_allocator

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o
	g++ mmap_file_pool.o test_allocator.o -o test_allocator

test_mmap_fixed: test_mmap_fixed.c
	gcc $(CFLAGS) test_mmap_fixed.c -o test_mmap_fixed

clean:
	rm -f test_allocator test_mmap_fixed testfile *.o

mmap_file_pool.o: mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h

test_allocator.o: mmap_file_pool.h mmap_allocator.h mmap_access_mode.h
