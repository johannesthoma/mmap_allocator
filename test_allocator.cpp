#include "mmap_allocator.h"
#include "mmap_exception.h"
#include "mmappable_vector.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <assert.h>
#include <memory>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;
using namespace mmap_allocator_namespace;

#define TESTFILE "testfile"
#define TESTFILE2 "testfile2"

void generate_test_file(int count, const char *fname)
{
	FILE *f;
	int i;

	f = fopen(fname, "w+");
	for (i=0;i<count;i++) {
		fwrite(&i, 1, sizeof(i), f);
	}
	fclose(f);
}

void test_test_file(int count, bool expect_zeros)
{
	FILE *f;
	int i, j;

	f = fopen(TESTFILE, "r");
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
		assert(0);
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message (expected): %s\n", e.what());
		assert(strcmp(e.what(), "Test") == 0);
	}
	try {
		do_throw();
		assert(0);
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message (expected): %s\n", e.what());
	}
}

/* Test exceptions: No file, mmap error */

void test_exceptions(void)
{
	bool exception_thrown;

	generate_test_file(1024, TESTFILE);
	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec(1024, 0, mmap_allocator<int>("", READ_ONLY));
			/* Default constructor used, allocate will fail */
		assert(0);
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message (expected): %s\n", e.what());
		exception_thrown = true;
	}
	assert(exception_thrown);

	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec_notexsting_file(1024, 0, mmap_allocator<int>("karin", READ_ONLY)); /* no such file or directory */
		assert(0);
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message (expected): %s\n", e.what());
		exception_thrown = true;
	}
	assert(exception_thrown);

	exception_thrown = false;
	try {
		vector<int, mmap_allocator<int> > int_vec_wrong_alignment_file(512, 0, mmap_allocator<int>(TESTFILE, READ_WRITE_PRIVATE, 123)); /* wrong alignment */
		/* No exception here expected */
	} catch (mmap_allocator_exception &e) {
		fprintf(stderr, "Exception message (not expected): %s\n", e.what());
		exception_thrown = true;
	}
	assert(!exception_thrown);
}

void test_mmap(void)
{
	int i;

	fprintf(stderr, "Testing int_vec_default\n");
	generate_test_file(1024, TESTFILE);
	mmappable_vector<int, mmap_allocator<int> > int_vec_default = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, DEFAULT_STL_ALLOCATOR, 0));
	int_vec_default.mmap_file(1024);
	assert(int_vec_default.size() == 1024);
	for (i=0;i<1024;i++) {
		int_vec_default[i] = i; /* no segfault */	
		assert(int_vec_default[i] == i); /* just to be sure */
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_rw_private\n");
	generate_test_file(1024, TESTFILE);
	mmappable_vector<int, mmap_allocator<int> > int_vec_rw_private = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_WRITE_PRIVATE, 0));
	int_vec_rw_private.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_rw_private[i] == i);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_ro\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_ro = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 0));
	int_vec_ro.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro[i] == i);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_shifted\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_shifted = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, sizeof(int)));
	int_vec_shifted.mmap_file(1024-1);
	for (i=0;i<1024-1;i++) {
		assert(int_vec_shifted[i] == i+1);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_pointer\n");
	mmappable_vector<int, mmap_allocator<int> > *int_vec_pointer = new mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 0));
	int_vec_pointer->mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert((*int_vec_pointer)[i] == i);
	}
	test_test_file(1024, false);
	delete int_vec_pointer;

	fprintf(stderr, "Testing int_vec_initialized_private\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_initialized_private = mmappable_vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>(TESTFILE, READ_WRITE_PRIVATE, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_private[i] == 0);
	}
	test_test_file(1024, false);

	fprintf(stderr, "Testing int_vec_initialized_shared\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_initialized_shared = mmappable_vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>(TESTFILE, READ_WRITE_SHARED, 0));
	for (i=0;i<1024;i++) {
		assert(int_vec_initialized_shared[i] == 0);
	}
	test_test_file(1024, true);

	fprintf(stderr, "Testing int_vec_big\n");
	generate_test_file(1024*1024, TESTFILE);
	mmappable_vector<int, mmap_allocator<int> > int_vec_big = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 0, MAP_WHOLE_FILE | ALLOW_REMAP));
	int_vec_big.mmap_file(1024*1024);
	for (i=0;i<1024*1024;i++) {
if (int_vec_big[i] != i) { fprintf(stderr, "falsch: i=%d val=%d\n", i, int_vec_big[i]); }
		assert(int_vec_big[i] == i);
	}
	test_test_file(1024*1024, false);

	fprintf(stderr, "Testing int_vec_shifted_big\n");
	generate_test_file(1024*1024, TESTFILE);
	mmappable_vector<int, mmap_allocator<int> > int_vec_shifted_big = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, sizeof(i), MAP_WHOLE_FILE | ALLOW_REMAP));
	int_vec_shifted_big.mmap_file(1024*1024-1);
	for (i=0;i<1024*1024-1;i++) {
		assert(int_vec_shifted_big[i] == i+1);
	}
	test_test_file(1024*1024, false);

	fprintf(stderr, "Testing int_vec_big_minus_one\n");
	generate_test_file(1024*1024-1, TESTFILE);
	mmappable_vector<int, mmap_allocator<int> > int_vec_big_minus_one = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 0, MAP_WHOLE_FILE | ALLOW_REMAP));
	int_vec_big_minus_one.mmap_file(1024*1024-1);
	for (i=0;i<1024*1024-1;i++) {
		assert(int_vec_big_minus_one[i] == i);
	}
	test_test_file(1024*1024-1, false);
}

void test_conversion(void)
{
	mmappable_vector<int> mmap_vector;
	vector<int> std_vector;
	mmappable_vector<int> mmap_vector2;
	int i;

	fprintf(stderr, "Testing conversion between STL vector and mmap vector.\n");
	generate_test_file(1024, TESTFILE);
	mmap_vector.mmap_file(TESTFILE, READ_ONLY, 0, 1024);
	for (i=0;i<1024;i++) {
		assert(mmap_vector[i] == i);
	}

	std_vector = to_std_vector(mmap_vector);
	for (i=0;i<1024;i++) {
		assert(std_vector[i] == i);
	}
	for (i=0;i<1024;i++) {
		std_vector[i] *= 2;
	}
	mmap_vector2 = to_mmappable_vector(std_vector);
	for (i=0;i<1024;i++) {
		assert(mmap_vector2[i] == i*2);
	}
}


void test_mmap_file_pool(void)
{
	generate_test_file(1024, TESTFILE);
	int *f = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 1024, false, false);
	int *f2 = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 1024, false, false);
	int i;

	assert(f == f2);

	for (i=0;i<1024;i++) {
		assert(f[i] == i);
	}
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 1024);
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 1024);
}

void test_mapping_smaller_area(void)
{
	fprintf(stderr, "Testing mapping of areas that fit in already mapped areas\n");
	generate_test_file(2048, TESTFILE);

	int *f = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 8192, false, false);
	int *first_page = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 4096, false, false);
	int *second_page = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 4096, 4096, false, false);

	assert(f == first_page);
	assert(f+1024 == second_page);

	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 8192);
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 4096);
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 4096, 4096);
}


void test_mapping_smaller_area_whole_file_flag(void)
{
	fprintf(stderr, "Testing whole file flag\n");
	generate_test_file(2048, TESTFILE);

	int *f = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 1, true, false);
	int *first_page = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 4096, false, false);
	int *second_page = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 4096, 4096, false, false);

	assert(f == first_page);
	assert(f+1024 == second_page);

	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 8192);
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 4096);
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 4096, 4096);
}


void test_mapping_smaller_area_whole_file_flag_allocator(void)
{
	int i;

	fprintf(stderr, "Testing whole file flag via allocator\n");
	generate_test_file(2048, TESTFILE);

	fprintf(stderr, "Testing int_vec_ro\n");
	mmappable_vector<int, mmap_allocator<int> > int_vec_ro = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 0, MAP_WHOLE_FILE));
	int_vec_ro.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro[i] == i);
	}

	mmappable_vector<int, mmap_allocator<int> > int_vec_ro_second_page = mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 4096, MAP_WHOLE_FILE));
	int_vec_ro_second_page.mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert(int_vec_ro_second_page[i] == i+1024);
	}
}


void test_mapping_smaller_area_whole_file_flag_allocator_deleting_first_vec(void)
{
	int i;

	fprintf(stderr, "Testing whole file flag via allocator, deleting first mapping before allocating second page\n");
	generate_test_file(2048, TESTFILE);

	fprintf(stderr, "Testing int_vec_ro\n");
	mmappable_vector<int, mmap_allocator<int> > *int_vec_ro_p = new mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 0, MAP_WHOLE_FILE));
	int_vec_ro_p->mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert((*int_vec_ro_p)[i] == i);
	}
	delete int_vec_ro_p;

	mmappable_vector<int, mmap_allocator<int> > *int_vec_ro_second_page_p = new mmappable_vector<int, mmap_allocator<int> >(mmap_allocator<int>(TESTFILE, READ_ONLY, 4096, MAP_WHOLE_FILE));
	int_vec_ro_second_page_p->mmap_file(1024);
	for (i=0;i<1024;i++) {
		assert((*int_vec_ro_second_page_p)[i] == i+1024);
	}
	delete int_vec_ro_second_page_p;
}


void test_new_interface(void)
{
	int i;

	fprintf(stderr, "Testing new interface\n");

	generate_test_file(1024, TESTFILE);

	mmappable_vector<int> vec;
	vec.mmap_file(TESTFILE, READ_ONLY, 0, 1024);

	for (i=0;i<1024;i++) {
		assert(vec[i] == i);
	}
	try {
		vec.mmap_file(TESTFILE, READ_ONLY, 0, 1024);
		assert(0);
	} catch (mmap_allocator_exception e) {
		fprintf(stderr, "Exception message (expected): %s\n", e.what());
	}
	vec.munmap_file();

	generate_test_file(2048, TESTFILE);
	vec.mmap_file(TESTFILE, READ_ONLY, 4096, 1024);
	for (i=0;i<1024;i++) {
		assert(vec[i] == i+1024);
	}
}

void test_cache_bug(void)
{
	mmappable_vector<int> vec;
	int i;
	
	fprintf(stderr, "Testing if wrong offset bug in pool is fixed.\n");
	generate_test_file(2048, TESTFILE);
	vec.mmap_file(TESTFILE, READ_ONLY, 4096, 1024);

	for (i=0;i<1024;i++) {
		assert(vec[i] == i+1024);
	}
}

void test_private_file_pool(void)
{
	mmappable_vector<int> vec;
	mmappable_vector<int> vec2;
	int i;
	
	fprintf(stderr, "Testing if bypass_file_pool works.\n");
	generate_test_file(1024, TESTFILE);
	vec.mmap_file(TESTFILE, READ_ONLY, 0, 1024, BYPASS_FILE_POOL);
	for (i=0;i<1024;i++) {
		assert(vec[i] == i);
	}

	vec2.mmap_file(TESTFILE, READ_ONLY, 0, 1024, BYPASS_FILE_POOL);
	for (i=0;i<1024;i++) {
		assert(vec[i] == i);
	}
	assert(&vec[0] != &vec2[0]);
}

#define FILESIZE (1024*1024*16)

void read_large_file(enum access_mode mode)
{
	struct timeval t, t2;
	mmappable_vector<int> vec;
	int i;

	gettimeofday(&t, NULL);
	
	vec.mmap_file(TESTFILE, mode, 0, FILESIZE);
	for (i=0;i<FILESIZE;i++) {
		assert(vec[i] == i);
	}
	gettimeofday(&t2, NULL);
	fprintf(stderr, "Mode: %d Time: %lu.%06lu\n", mode, (t2.tv_sec - t.tv_sec)-(t2.tv_usec < t.tv_usec), (t2.tv_usec < t.tv_usec)*1000000 + (t2.tv_usec - t.tv_usec));
}

void test_large_file(void)
{
	fprintf(stderr, "Testing large file.\n");
	generate_test_file(FILESIZE, TESTFILE); /* 1G */

	read_large_file(READ_ONLY);
	read_large_file(READ_WRITE_PRIVATE);
	read_large_file(READ_WRITE_SHARED);
}

void test_multiple_open(void)
{
	generate_test_file(1024, TESTFILE);
	generate_test_file(1024, TESTFILE2);
	mmappable_vector<int> vec1, vec2, vec3, vec4;

	fprintf(stderr, "Testing multiple open (you need to strace this).\n");
	vec1.mmap_file(TESTFILE, READ_ONLY, 0, 1024, MAP_WHOLE_FILE);
	vec2.mmap_file(TESTFILE, READ_ONLY, 0, 1024, MAP_WHOLE_FILE);
	vec3.mmap_file(TESTFILE2, READ_ONLY, 0, 1024, MAP_WHOLE_FILE);
	vec4.mmap_file(TESTFILE2, READ_ONLY, 0, 1024, MAP_WHOLE_FILE);
}

void test_keep_forever(void)
{
	generate_test_file(1024, TESTFILE);
	mmappable_vector<int> vec1, vec2, vec3, vec4;

	fprintf(stderr, "Testing multiple open (you need to strace this).\n");
	{
		mmappable_vector<int> vec;
		fprintf(stderr, "Testing mapping without KEEP_FOREVER (you need to strace this: there should be a close).\n");
		vec.mmap_file(TESTFILE, READ_ONLY, 0, 1024, MAP_WHOLE_FILE);
	}
	{
		mmappable_vector<int> vec;
		fprintf(stderr, "Testing mapping without KEEP_FOREVER (you need to strace this: the file should be reopened).\n");
		vec.mmap_file(TESTFILE, READ_ONLY, 0, 1024, MAP_WHOLE_FILE);
	}
	{
		mmappable_vector<int> vec;
		fprintf(stderr, "Testing mapping with KEEP_FOREVER (you need to strace this: there should be NO close).\n");
		vec.mmap_file(TESTFILE, READ_ONLY, 0, 1024, MAP_WHOLE_FILE | KEEP_FOREVER);
	}
	{
		mmappable_vector<int> vec;
		fprintf(stderr, "Testing mapping with KEEP_FOREVER (you need to strace this: the file shouldn't be reopened).\n");
		vec.mmap_file(TESTFILE, READ_ONLY, 0, 1024, MAP_WHOLE_FILE | KEEP_FOREVER);
	}
}


void test_allocate_0_bytes(void) /* shouldn't segfault */
{
	fprintf(stderr, "Testing vectors of mmappable_vectors.\n");

	vector<mmappable_vector<int> > vecs;
	vecs.resize(2);
	for (int i=0; i<2; i++) {
		vecs[i].mmap_file(TESTFILE, READ_ONLY, 0, 1024, 0);
	        for (int j=0;j<1024;j++) {
			assert(vecs[i][j] == j);
	        }
	}
}


int main(int argc, char ** argv)
{
	set_verbosity(1);

	test_page_align_macros();
	test_throw_catch();
	test_exceptions();
	test_mmap_file_pool();
	test_mmap();
	test_conversion();
	test_cache_bug();
	test_mapping_smaller_area();
	test_mapping_smaller_area_whole_file_flag();
	test_mapping_smaller_area_whole_file_flag_allocator();
	test_mapping_smaller_area_whole_file_flag_allocator_deleting_first_vec();
	test_new_interface();
	test_private_file_pool();
	test_large_file();
	test_multiple_open();
	test_keep_forever();
	test_allocate_0_bytes();
}
