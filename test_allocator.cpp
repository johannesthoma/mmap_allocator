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

	fprintf(stderr, "Testing int_vec_default\n");
	generate_test_file(1024);
	mmappable_vector<int, mmap_allocator<int> > int_vec_default = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", DEFAULT_STL_ALLOCATOR, 0));
	int_vec_default.mmap_file(1024);
	assert(int_vec_default.size() == 1024);
	for (i=0;i<1024;i++) {
		int_vec_default[i] = i; /* no segfault */	
		assert(int_vec_default[i] == i); /* just to be sure */
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_rw_private\n");
	generate_test_file(1024);
	mmappable_vector<int, mmap_allocator<int> > int_vec_rw_private = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_WRITE_PRIVATE, 0));
	int_vec_rw_private.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_rw_private[i] == i);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_ro\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_ro = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_ro.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro[i] == i);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_shifted\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_shifted = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, sizeof(int)));
	int_vec_shifted.mmap_file(1024-1);
	for (i=0;i<1024-1;i++) {
		assert(int_vec_shifted[i] == i+1);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_pointer\n");
	mmappable_vector<int, mmap_allocator<int> > *int_vec_pointer = new mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0));
	int_vec_pointer->mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert((*int_vec_pointer)[i] == i);
	}
	test_test_file(1024, false);
	delete int_vec_pointer;

	fprintf(stderr, "Testing int_vec_initialized_private\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_initialized_private = mmappable_vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>("testfile", READ_WRITE_PRIVATE, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_private[i] == 0);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_initialized_shared\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_initialized_shared = mmappable_vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>("testfile", READ_WRITE_SHARED, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_shared[i] == 0);
	}
	test_test_file(1024, true);

	fprintf(stderr, "Testing int_vec_big\n");
	generate_test_file(1024*1024);
	mmappable_vector<int, mmap_allocator<int> > int_vec_big = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0, true, true));
	int_vec_big.mmap_file(1024*1024);
	for (i=0;i<1024*1024;i++) {
if (int_vec_big[i] != i) { fprintf(stderr, "falsch: i=%d val=%d\n", i, int_vec_big[i]); }
		assert(int_vec_big[i] == i);
	}
	test_test_file(1024*1024, false);

	fprintf(stderr, "Testing int_vec_shifted_big\n");
	generate_test_file(1024*1024);
	mmappable_vector<int, mmap_allocator<int> > int_vec_shifted_big = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, sizeof(i), true, true));
	int_vec_shifted_big.mmap_file(1024*1024-1);
	for (i=0;i<1024*1024-1;i++) {
		assert(int_vec_shifted_big[i] == i+1);
	}
	test_test_file(1024*1024, false);

	fprintf(stderr, "Testing int_vec_big_minus_one\n");
	generate_test_file(1024*1024-1);
	mmappable_vector<int, mmap_allocator<int> > int_vec_big_minus_one = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0, true, true));
	int_vec_big_minus_one.mmap_file(1024*1024-1);
	for (i=0;i<1024*1024-1;i++) {
		assert(int_vec_big_minus_one[i] == i);
	}
	test_test_file(1024*1024-1, false);
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

void test_mapping_smaller_area(void)
{
	fprintf(stderr, "Testing mapping of areas that fit in already mapped areas\n");
	generate_test_file(2048);

	int *f = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 0, 8192, false, false);
	int *first_page = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 0, 4096, false, false);
	int *second_page = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 4096, 4096, false, false);

	assert(f == first_page);
	assert(f+1024 == second_page);

	the_pool.munmap_file(string("testfile"), READ_ONLY, 0, 8192);
	the_pool.munmap_file(string("testfile"), READ_ONLY, 0, 4096);
	the_pool.munmap_file(string("testfile"), READ_ONLY, 4096, 4096);
}


void test_mapping_smaller_area_whole_file_flag(void)
{
	fprintf(stderr, "Testing whole file flag\n");
	generate_test_file(2048);

	int *f = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 0, 1, true, false);
	int *first_page = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 0, 4096, false, false);
	int *second_page = (int*)the_pool.mmap_file(string("testfile"), READ_ONLY, 4096, 4096, false, false);

	assert(f == first_page);
	assert(f+1024 == second_page);

	the_pool.munmap_file(string("testfile"), READ_ONLY, 0, 8192);
	the_pool.munmap_file(string("testfile"), READ_ONLY, 0, 4096);
	the_pool.munmap_file(string("testfile"), READ_ONLY, 4096, 4096);
}


void test_mapping_smaller_area_whole_file_flag_allocator(void)
{
	int i;

	fprintf(stderr, "Testing whole file flag via allocator\n");
	generate_test_file(2048);

	fprintf(stderr, "Testing int_vec_ro\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_ro = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 0, true, false));
	int_vec_ro.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro[i] == i);
	}

	mmappable_vector<int, mmap_allocator<int> > int_vec_ro_second_page = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>("testfile", READ_ONLY, 4096, true, false));
	int_vec_ro_second_page.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro_second_page[i] == i+1024);
	}
}


void test_new_interface(void)
{
	int i;

	fprintf(stderr, "Testing new interface\n");

	generate_test_file(1024);

	mmappable_vector<int> vec;
	vec.mmap_file("testfile", READ_ONLY, 0, 1024);

	for (i=0;i<1024;i++) {
		assert(vec[i] == i);
	}
	try {
		vec.mmap_file("testfile", READ_ONLY, 0, 1024);
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message: %s\n", e.message());
	}
	vec.munmap_file();
}

int main(int argc, char ** argv)
{
	test_page_align_macros();
	test_throw_catch();
	test_exceptions();
	test_mmap_file_pool();
	test_mmap();
	test_conversion();
	test_mapping_smaller_area();
	test_mapping_smaller_area_whole_file_flag();
	test_mapping_smaller_area_whole_file_flag_allocator();
	test_new_interface();
}
