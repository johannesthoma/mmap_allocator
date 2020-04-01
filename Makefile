CXXFLAGS=-Wall -fPIC -O2
CFLAGS=-Wall -fPIC -O2

# Enable to test with GCC 3.4
# CXX=g++34

PREFIX=/usr

SOURCES=mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h

HEADERS=mmap_access_mode.h mmap_allocator.h mmap_file_pool.h mmap_exception.h mmappable_vector.h
LIBRARIES=libmmap_allocator.so libmmap_allocator.a

SRC_INSTALL_TARGET_DIR=/home/johannes/re3

all: $(LIBRARIES)

debug: CXXFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1 -g
debug: CFLAGS+=-DMMAP_ALLOCATOR_DEBUG=1 -g
debug: clean all

libmmap_allocator.so: mmap_file_pool.o
	$(CXX) -shared -o libmmap_allocator.so mmap_file_pool.o

libmmap_allocator.a: mmap_file_pool.o
	ar r libmmap_allocator.a mmap_file_pool.o

install_sources: $(SOURCES)
	cp $(SOURCES) $(SRC_INSTALL_TARGET_DIR)

install: all
	install -Dm 644 -t $(DESTDIR)$(PREFIX)/include $(HEADERS)
	install -Dm 755 -t $(DESTDIR)$(PREFIX)/lib $(LIBRARIES)

test: test_allocator
	@echo "Running mmap allocator regression test suite."
	bash -c 'export LD_LIBRARY_PATH=. ; ./test_allocator'

debugtest: debug test_allocator
	@echo "Running mmap allocator regression test suite with verbose enabled."
	bash -c 'export LD_LIBRARY_PATH=. ; ./test_allocator'

test_allocator: mmap_allocator.h mmap_file_pool.o test_allocator.o $(LIBRARIES)
	$(CXX) test_allocator.o -L. -lmmap_allocator -o test_allocator

test_mmap_fixed: test_mmap_fixed.c
	$(CC) $(CFLAGS) test_mmap_fixed.c -o test_mmap_fixed

clean:
	rm -f test_allocator test_mmap_fixed testfile testfile2 *.o $(LIBRARIES)

mmap_file_pool.o: mmap_file_pool.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h

test_allocator.o: test_allocator.cpp mmap_file_pool.h mmap_allocator.h mmap_access_mode.h mmappable_vector.h mmap_exception.h
