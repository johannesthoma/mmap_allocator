#include "mmap_allocator.h"
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <memory>

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

	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec(1024, 0, mmap_allocator<int>());
			/* Default constructor used, allocate will fail */
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
		exception_thrown = true;
	}
	assert(exception_thrown);

	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec_notexsting_file(1024, 0, mmap_allocator<int>("karin")); /* no such file or directory */
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

	fprintf(stderr, "Testing int_vec_ro\n");
	vector<int, mmap_allocator<int> > int_vec_ro = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_ro.reserve(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro[i] == i);
	}
	test_test_file(1024, false);

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

	fprintf(stderr, "Testing int_vec_big\n");
	generate_test_file(1024*1024);
	vector<int, mmap_allocator<int> > int_vec_big = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_big.reserve(1024*1024);
	for (i=0;i<1024*1024;i++) {
		assert(int_vec_big[i] == i);
	}
	test_test_file(1024*1024, false);

	fprintf(stderr, "Testing int_vec_shifted_big\n");
	generate_test_file(1024*1024);
	vector<int, mmap_allocator<int> > int_vec_shifted_big = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, sizeof(i)));
	int_vec_shifted_big.reserve(1024*1024-1);
	for (i=0;i<1024*1024-1;i++) {
		assert(int_vec_shifted_big[i] == i+1);
	}
	test_test_file(1024*1024, false);
}


int main(int argc, char ** argv)
{
	test_page_align_macros();
	test_throw_catch();
	test_exceptions();
	test_mmap();
}
