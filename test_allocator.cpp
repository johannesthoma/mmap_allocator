#include "mmap_allocator.h"
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <memory>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace mmap_allocator_namespace;

void generate_test_file(int count)
{
	FILE *f;
	int i;

	f = fopen("testfile", "w+");
	for (i=0;i<count;i++) {
		fwrite(&i, 1, sizeof(i), f);
	}
	fclose(f);
}

void test_test_file(int count, bool expect_zeros)
{
	FILE *f;
	int i, j;

	f = fopen("testfile", "r");
	for (i=0;i<count;i++) {
		fread(&j, 1, sizeof(j), f);
		assert(j == (expect_zeros ? 0 : i));
	}
	fclose(f);
}

void do_throw(void)
{
	throw mmap_allocator_exception("Test2");
}

void test_page_align_macros(void)
{
	int p;

	p = 0x8000;
	assert(ALIGN_TO_PAGE(p) == 0x8000);
	assert(UPPER_ALIGN_TO_PAGE(p) == 0x8000);
	assert(OFFSET_INTO_PAGE(p) == 0x0);

	p = 0x64ab;
	assert(ALIGN_TO_PAGE(p) == 0x6000);
	assert(UPPER_ALIGN_TO_PAGE(p) == 0x7000);
	assert(OFFSET_INTO_PAGE(p) == 0x4ab);
}

void test_throw_catch(void)
{
	try {
		throw mmap_allocator_exception("Test");
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
	}
	try {
		do_throw();
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
	}
}

/* Test exceptions: No file, mmap error */

void test_exceptions(void)
{
	bool exception_thrown;

	generate_test_file(1024);
	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec(1024, 0, mmap_allocator<int>("", READ_ONLY));
			/* Default constructor used, allocate will fail */
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
		exception_thrown = true;
	}
	assert(exception_thrown);

	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec_notexsting_file(1024, 0, mmap_allocator<int>("karin", READ_ONLY)); /* no such file or directory */
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
		exception_thrown = true;
	}
	assert(exception_thrown);

	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec_wrong_alignment_file(512, 0, mmap_allocator<int>("testfile", READ_WRITE_PRIVATE, 123)); /* wrong alignment */
	} catch (mmap_allocator_exception &e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
		exception_thrown = true;
	}
	assert(!exception_thrown);
}

void test_mmap(void)
{
	int i;

#if 0
	fprintf(stderr, "Testing int_vec_default\n");
	generate_test_file(1024);
	vector<int, mmap_allocator<int> > int_vec_default = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", DEFAULT_STL_ALLOCATOR, 0));
	int_vec_default.reserve(1024);
	for (i=0;i<1024;i++) {
		int_vec_default[i] = i; /* no segfault */	
		assert(int_vec_default[i] == i); /* just to be sure */
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_rw_private\n");
	generate_test_file(1024);
	vector<int, mmap_allocator<int> > int_vec_rw_private = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_WRITE_PRIVATE, 0));
	int_vec_rw_private.reserve(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_rw_private[i] == i);
	}
	test_test_file(1024, false);
#endif


	fprintf(stderr, "Testing int_vec_ro\n");
	vector<int, mmap_allocator<int> > int_vec_ro = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_ro.reserve(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro[i] == i);
	}
	test_test_file(1024, false);

#if 0
	fprintf(stderr, "Testing int_vec_shifted\n");
	vector<int, mmap_allocator<int> > int_vec_shifted = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, sizeof(int)));
	int_vec_shifted.reserve(1024-1);
	for (i=0;i<1024-1;i++) {
		assert(int_vec_shifted[i] == i+1);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_pointer\n");
	vector<int, mmap_allocator<int> > *int_vec_pointer = new vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_pointer->reserve(1024);
	for (i=0;i<1024;i++) {
		assert((*int_vec_pointer)[i] == i);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_initialized_private\n");
	vector<int, mmap_allocator<int> > int_vec_initialized_private = vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>("testfile", READ_WRITE_PRIVATE, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_private[i] == 0);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_initialized_shared\n");
	vector<int, mmap_allocator<int> > int_vec_initialized_shared = vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>("testfile", READ_WRITE_SHARED, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_shared[i] == 0);
	}
	test_test_file(1024, true);
#endif 

	fprintf(stderr, "Testing int_vec_big\n");
	generate_test_file(1024*1024);
	vector<int, mmap_allocator<int> > int_vec_big = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_big.reserve(1024*1024);
	for (i=0;i<1024*1024;i++) {
if (int_vec_big[i] != i) { fprintf(stderr, "falsch: i=%d val=%d\n", i, int_vec_big[i]); }
		assert(int_vec_big[i] == i);
	}
	test_test_file(1024*1024, false);

#if 0
	fprintf(stderr, "Testing int_vec_shifted_big\n");
	generate_test_file(1024*1024);
	vector<int, mmap_allocator<int> > int_vec_shifted_big = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, sizeof(i)));
	int_vec_shifted_big.reserve(1024*1024-1);
	for (i=0;i<1024*1024-1;i++) {
		assert(int_vec_shifted_big[i] == i+1);
	}
	test_test_file(1024*1024, false);

	fprintf(stderr, "Testing int_vec_big_minus_one\n");
	generate_test_file(1024*1024-1);
	vector<int, mmap_allocator<int> > int_vec_big_minus_one = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_big_minus_one.reserve(1024*1024-1);
	for (i=0;i<1024*1024-1;i++) {
		assert(int_vec_big_minus_one[i] == i);
	}
	test_test_file(1024*1024-1, false);
#endif 
}

void test_conversion(void)
{
	vector<int, mmap_allocator<int> > mmap_vector = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", DEFAULT_STL_ALLOCATOR, 1000));

	vector<int> std_vector;

	std_vector = to_std_vector(mmap_vector);
	mmap_vector = to_mmap_vector(std_vector);
}


void test_mmap_file_pool(void)
{
	generate_test_file(1024);
	int *f = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 0, 1024, false, false);
	int *f2 = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 0, 1024, false, false);
	int i;

	assert(f == f2);

	for (i=0;i<1024;i++) {
		assert(f[i] == i);
	}
	the_pool.munmap_file(string("testfile"), READ_ONLY, 0, 1024);
	the_pool.munmap_file(string("testfile"), READ_ONLY, 0, 1024);
}

void test_mmap_directly()
{
	int fd;
	void *area_short, *area_long;

	generate_test_file(1024);
	fd = open("testfile", O_RDONLY);
	assert(fd>=0);
	
	area_short = mmap(0, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
	assert(area_short != MAP_FAILED);
	
	generate_test_file(1024*1024);
	area_long = mmap(0, 4096*1024, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0);
	assert(area_long != MAP_FAILED);
	assert(area_short == area_long);
}

int main(int argc, char ** argv)
{
	test_page_align_macros();
	test_throw_catch();
	test_exceptions();
	test_mmap_file_pool();
	test_mmap_directly();
//	test_mmap();
	test_conversion();
}
