#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

void test_mmap_directly()
{
	int fd;
	void *area_short, *area_long;

	generate_test_file(1024);
	fd = open("testfile", O_RDONLY);
	assert(fd>=0);
	
	area_short = mmap(0, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
	assert(area_short != MAP_FAILED);
	
	close(fd);
	generate_test_file(1024*1024);
	fd = open("testfile", O_RDONLY);
	assert(fd>=0);

	area_long = mmap(area_short, 4096*1024, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0);
	assert(area_long != MAP_FAILED);
	assert(area_short == area_long);
}

int main(int argc, char ** argv)
{
	test_mmap_directly();
/* Crashes on shutdown for libc 2.5 and kernel 2.6.18 (CentOS 5) */
}
