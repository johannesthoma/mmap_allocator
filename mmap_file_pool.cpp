#include "mmap_file_pool.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace mmap_allocator_namespace {
	mmap_file_identifier::mmap_file_identifier(std::string fname, enum access_mode access_mode_param)
	{
		struct stat buf;
		if (stat(fname.c_str(), &buf) < 0) {
#ifdef MMAP_ALLOCATOR_DEBUG
			perror("stat");
#endif
			throw mmap_allocator_exception("Cannot stat file");
		}

		device = buf.st_dev;
		inode = buf.st_ino;
		access_mode = access_mode_param;
	}

	mmap_file_identifier::mmap_file_identifier(const mmap_file_identifier &other)
	{
		device = other.device;
		inode = other.inode;
		access_mode = other.access_mode;
	}

	bool mmap_file_identifier::operator==(const mmap_file_identifier &other) const
	{
		return device == other.device &&
			inode == other.inode &&
			access_mode == other.access_mode;
	}
	
	bool mmap_file_identifier::operator<(const mmap_file_identifier &other) const
	{
		return inode < other.inode ||
			(inode == other.inode && access_mode < other.access_mode) ||
			(inode == other.inode && access_mode == other.access_mode && device < other.device);
			
	}
	
	void mmapped_file::open_and_mmap_file(std::string filename, enum access_mode access_mode, off_t offset, size_t length)
	{
		if (filename.c_str()[0] == '\0') {
			throw mmap_allocator_exception("mmap_allocator not correctly initialized: filename is empty.");
		}
		int mode;
		int prot;
		int mmap_mode;

		switch (access_mode) {
		case READ_ONLY: mode = O_RDONLY; prot = PROT_READ; mmap_mode = MAP_SHARED; break;
		case READ_WRITE_SHARED: mode = O_RDWR; prot = PROT_READ | PROT_WRITE; mmap_mode = MAP_SHARED; break;
		case READ_WRITE_PRIVATE: mode = O_RDWR; prot = PROT_READ | PROT_WRITE; mmap_mode = MAP_PRIVATE; break;
		default: throw mmap_allocator_exception("Internal error"); break;
		}

		fd = open(filename.c_str(), mode);
		if (fd < 0) {
#ifdef MMAP_ALLOCATOR_DEBUG
			perror("open");
#endif
			throw mmap_allocator_exception("Error opening file");
		}
		memory_area = mmap(NULL, UPPER_ALIGN_TO_PAGE(length), prot, mmap_mode, fd, ALIGN_TO_PAGE(offset));
		if (memory_area == MAP_FAILED) {
#ifdef MMAP_ALLOCATOR_DEBUG
			perror("mmap");
#endif
			throw mmap_allocator_exception("Error in mmap");
		}
		offset_mapped = ALIGN_TO_PAGE(offset);
		size_mapped = length;
	}

	void mmapped_file::munmap_and_close_file(void)
	{
		if (munmap(memory_area, size_mapped) < 0) {
#ifdef MMAP_ALLOCATOR_DEBUG
			perror("munmap");
#endif
			throw mmap_allocator_exception("Error in munmap");
		}
		if (close(fd)) {
#ifdef MMAP_ALLOCATOR_DEBUG
			perror("close");
#endif
			throw mmap_allocator_exception("Error in close");
		}
	}

	void *mmap_file_pool::mmap_file(std::string fname, enum access_mode access_mode, off_t offset, size_t length)
	{
		mmap_file_identifier the_identifier(fname, access_mode);
		mmapped_file_map_t::iterator it;
		mmapped_file the_file;

		it = the_map.find(the_identifier);
		if (it != the_map.end()) {
			the_file = it->second;
		} else {
			the_file.open_and_mmap_file(fname, access_mode, offset, length);
			the_map.insert(mmapped_file_pair_t(the_identifier, the_file));
		}
		return the_file.get_memory_area();
	}

	void mmap_file_pool::munmap_file(std::string fname, enum access_mode access_mode, off_t offset, size_t length)
	{
		mmap_file_identifier the_identifier(fname, access_mode);
		mmapped_file_map_t::iterator it;
		mmapped_file the_file;

		it = the_map.find(the_identifier);
		if (it != the_map.end()) {
			the_file = it->second;
		} else {
			throw mmap_allocator_exception("File not found in pool");
		}

		the_file.munmap_and_close_file();
		the_map.erase(it);
	}

	mmap_file_pool the_pool; /* TODO: move to app object */
}
