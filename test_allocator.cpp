// #include "mmap_allocator.h"
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <memory>

using namespace std;

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

	vector<int> int_vec(1024, 0, allocator<int>());
//	vector<int> int_vec(1024);
	int i;
	for (i=0;i<1024;i++) {
		assert(int_vec[i] == i);
	}

	return 0;
}
