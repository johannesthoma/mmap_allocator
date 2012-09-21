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

fprintf(stderr, "karin\n");
	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec_notexsting_file(1024, 0, mmap_allocator<int>("karin", 0)); /* no such file or directory */
	// } catch (mmap_allocator_exception e) {
	} catch (...) {
//		fprintf(stderr, "Exception message: %s\n", e.message());
		exception_thrown = true;
	}
	assert(exception_thrown);
fprintf(stderr, "zak\n");

	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec_notexsting_file(1024, 0, mmap_allocator<int>("testfile", 123)); /* wrong alignment */
	} catch (mmap_allocator_exception &e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
		exception_thrown = true;
	}
	assert(exception_thrown);
}


int main(int argc, char ** argv)
{
	generate_test_file();
	test_throw_catch();
	test_exceptions();

	vector<int, mmap_allocator<int> > int_vec(1024, 0, mmap_allocator<int>());
	vector<int> test_vec;
	vector<int> test_vec2;

	test_vec = test_vec2;

	mmap_allocator<int> a;
	a.allocate(10);

/*
	int i;
	for (i=0;i<1024;i++) {
		assert((*int_vec_pointer)[i] == i);
	}
*/

	vector<int, mmap_allocator<int> > *int_vec_pointer = new vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>());

	return 0;
}
