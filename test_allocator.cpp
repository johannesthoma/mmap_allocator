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


int main(int argc, char ** argv)
{
	generate_test_file();

	vector<int, mmap_allocator<int> > int_vec(1024, 0, mmap_allocator<int>());
	vector<int> test_vec;
	vector<int> test_vec2;

	test_vec = test_vec2;

	mmap_allocator<int> a;
	a.allocate(10);

	vector<int, mmap_allocator<int> > *int_vec_pointer = new vector<int, mmap_allocator<int> >(1024, 0, mmap_allocator<int>());
/*
	int i;
	for (i=0;i<1024;i++) {
		assert((*int_vec_pointer)[i] == i);
	}
*/

	return 0;
}
