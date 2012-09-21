#include "mmap_allocator.h"
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <memory>

using namespace std;
using namespace mmap_allocator_namespace;

void generate_test_file(void)
{
	FILE *f;
	int i;

	f = fopen("testfile", "w+");
	for (i=0;i<1024;i++) {
		fwrite(&i, 1, sizeof(i), f);
	}
	fclose(f);
}

void test_test_file(bool expect_zeros)
{
	FILE *f;
	int i, j;

	f = fopen("testfile", "r");
	for (i=0;i<1024;i++) {
		fread(&j, 1, sizeof(j), f);
		assert(j == (expect_zeros ? 0 : i));
	}
	fclose(f);
}

void do_throw(void)
{
	throw mmap_allocator_exception("Test2");
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
		vector<int, mmap_allocator<int> > int_vec_notexsting_file(1024, 0, mmap_allocator<int>("testfile", READ_ONLY, 123)); /* wrong alignment */
	} catch (mmap_allocator_exception &e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
		exception_thrown = true;
	}
	assert(exception_thrown);
}

void test_mmap(void)
{
	int i;

	fprintf(stderr, "Testing int_vec_rw_private\n");
	generate_test_file();
	vector<int, mmap_allocator<int> > int_vec_rw_private = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_WRITE_PRIVATE, 0));
	int_vec_rw_private.reserve(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_rw_private[i] == i);
	}
	test_test_file(false);

	fprintf(stderr, "Testing int_vec_ro\n");
	vector<int, mmap_allocator<int> > int_vec_ro = vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_ro.reserve(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro[i] == i);
	}
	test_test_file(false);

	fprintf(stderr, "Testing int_vec_pointer\n");
	vector<int, mmap_allocator<int> > *int_vec_pointer = new vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_pointer->reserve(1024);
	for (i=0;i<1024;i++) {
		assert((*int_vec_pointer)[i] == i);
	}
	test_test_file(false);

	fprintf(stderr, "Testing int_vec_initialized_private\n");
	vector<int, mmap_allocator<int> > int_vec_initialized_private = vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>("testfile", READ_WRITE_PRIVATE, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_private[i] == 0);
	}
	test_test_file(false);

	fprintf(stderr, "Testing int_vec_initialized_shared\n");
	vector<int, mmap_allocator<int> > int_vec_initialized_shared = vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>("testfile", READ_WRITE_SHARED, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_shared[i] == 0);
	}
	test_test_file(true);
}


int main(int argc, char ** argv)
{
	generate_test_file();
	test_throw_catch();
	test_exceptions();
	test_mmap();
}
