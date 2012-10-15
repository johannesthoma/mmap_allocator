CPPFLAGS=-g -Wall
CFLAGS=-g -Wall

SOURCES=mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmap_file_pool_windows_dummy.cpp

SRC_INSTALL_TARGET_DIR=/home/johannes/re3

all: test_allocator mmap_file_pool.o

debug: CPPFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: CFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1
debug: clean all

install_sources: $(SOURCES)
	cp $(SOURCES) $(SRC_INSTALL_TARGET_DIR)

# install: all
	
test: all
	@echo "Running mmap allocator regression test suite."
	./test_allocator

debugtest: debug
	@echo "Running mmap allocator regression test suite with verbose enabled."
	./test_allocator

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o
	g++ mmap_file_pool.o test_allocator.o -o test_allocator

test_mmap_fixed: test_mmap_fixed.c
	gcc $(CFLAGS) test_mmap_fixed.c -o test_mmap_fixed

clean:
	rm -f test_allocator test_mmap_fixed testfile *.o

mmap_file_pool.o: mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h

test_allocator.o: test_allocator.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h
