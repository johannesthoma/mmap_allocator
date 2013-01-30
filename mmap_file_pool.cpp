#include "mmap_file_pool.h"
#include "mmap_exception.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>

namespace mmap_allocator_namespace {

#ifndef MMAP_ALLOCATOR_DEBUG
#define MMAP_ALLOCATOR_DEBUG 0
#endif

	int verbosity = MMAP_ALLOCATOR_DEBUG;

	void set_verbosity(int v)
	{
		verbosity = v;
	}

	int get_verbosity(void)
	{
		return verbosity;
	}

	off_t filesize(int fd, std::string fname)
	{
		struct stat buf;
		if (fstat(fd, &buf) < 0) {
			if (get_verbosity() > 0) {
				perror("stat");
			}

			throw mmap_allocator_exception("Cannot stat file" + fname);
		}

		return buf.st_size;
	}

	mmap_file_identifier::mmap_file_identifier(std::string fname, enum access_mode access_mode_param)
	{
		struct stat buf;
		if (stat(fname.c_str(), &buf) < 0) {
			if (get_verbosity() > 0) {
				perror("stat");
			}
			throw mmap_allocator_exception("Cannot stat file "+fname);
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
	
	void *mmapped_file::open_and_mmap_file(std::string filename, enum access_mode access_mode, off_t offset, size_t length, bool map_whole_file, bool allow_remap)
	{
		if (filename.c_str()[0] == '\0') {
			throw mmap_allocator_exception("mmap_allocator not correctly initialized: filename is empty.");
		}
		int mode;
		int prot;
		int mmap_mode = 0;
		off_t offset_to_map;
		size_t length_to_map;
		void *address_to_map = NULL;

		if (memory_area != NULL) {
			address_to_map = memory_area;

	/* do not use MAP_FIXED, since that may invalidate other memory
           areas in the process, such as shared libraries, which would 
           lead to a mystic Segfault. */

		}
		switch (access_mode) {
		case READ_ONLY: mode = O_RDONLY; prot = PROT_READ; mmap_mode |= MAP_SHARED; break;
		case READ_WRITE_SHARED: mode = O_RDWR; prot = PROT_READ | PROT_WRITE; mmap_mode |= MAP_SHARED; break;
		case READ_WRITE_PRIVATE: mode = O_RDONLY; prot = PROT_READ | PROT_WRITE; mmap_mode |= MAP_PRIVATE; break;
		default: throw mmap_allocator_exception("Internal error"); break;
		}

		if (fd == -1) {
			fd = open(filename.c_str(), mode);
			if (fd < 0) {
				if (get_verbosity() > 0) {
					perror("open");
				}

				throw mmap_allocator_exception("Error opening file " + filename);
			}
		}
		if (map_whole_file) {
			offset_to_map = 0;
			length_to_map = filesize(fd, filename);
		} else {
			offset_to_map = ALIGN_TO_PAGE(offset);
			length_to_map = UPPER_ALIGN_TO_PAGE(length);
		}

		if (offset_to_map == offset_mapped && length_to_map == size_mapped) {
			reference_count++;
			return ((char*)memory_area)+offset-offset_mapped;
		}
		if (offset_to_map >= offset_mapped && length_to_map + offset_to_map - offset_mapped <= size_mapped)
		{
			reference_count++;
			return ((char*)memory_area)+offset-offset_mapped;
		}
		
		if (memory_area != NULL) {
			if (munmap(memory_area, size_mapped) < 0) {
				if (get_verbosity() > 0) {
					perror("munmap");
				}
				throw mmap_allocator_exception("Error in munmap file "+filename);
			}
		}

		memory_area = mmap(address_to_map, length_to_map, prot, mmap_mode, fd, offset_to_map);
		if (address_to_map != NULL && !allow_remap && memory_area != MAP_FAILED && memory_area != address_to_map) {
			if (munmap(memory_area, length_to_map) < 0) {
				if (get_verbosity() > 0) {
					perror("munmap");
				}
				throw mmap_allocator_exception("Error in munmap" + filename);
			}
			throw mmap_allocator_exception("Request to remap area but allow_remap is not given (remapping "+filename+")");
		}

		if (memory_area == MAP_FAILED) {
			if (get_verbosity() > 0) {
				perror("mmap");
			}
			throw mmap_allocator_exception("Error in mmap "+filename);
		}
		offset_mapped = offset_to_map;
		size_mapped = length_to_map;
		reference_count++;

		void *ret = ((char*)memory_area)+offset-offset_to_map;
		// assert(ret >= memory_area && ret < (char*)memory_area+size_mapped);

		return ret;
	}

	bool mmapped_file::munmap_and_close_file(void)
	{
		reference_count--;
		if (reference_count > 0) {
			return false;
		}
		if (munmap(memory_area, size_mapped) < 0) {
			if (get_verbosity() > 0) {
				perror("munmap");
			}
			throw mmap_allocator_exception("Error in munmap");
		}
		if (close(fd)) {
			if (get_verbosity() > 0) {
				perror("close");
			}
			throw mmap_allocator_exception("Error in close");
		}
		fd = -1;
		return true;
	}

	void *mmap_file_pool::mmap_file(std::string fname, enum access_mode access_mode, off_t offset, size_t length, bool map_whole_file, bool allow_remap)
	{
		mmap_file_identifier the_identifier(fname, access_mode);
		mmapped_file_map_t::iterator it;

		it = the_map.find(the_identifier);
		if (it != the_map.end()) {
			return it->second.open_and_mmap_file(fname, access_mode, offset, length, map_whole_file, allow_remap);
		} else {
			mmapped_file the_file;
			void *ret;

			ret = the_file.open_and_mmap_file(fname, access_mode, offset, length, map_whole_file, allow_remap);
			the_map.insert(mmapped_file_pair_t(the_identifier, the_file));
			return ret;
		}
	}

	void mmap_file_pool::munmap_file(std::string fname, enum access_mode access_mode, off_t offset, size_t length)
	{
		mmap_file_identifier the_identifier(fname, access_mode);
		mmapped_file_map_t::iterator it;

		it = the_map.find(the_identifier);
		if (it != the_map.end()) {
			if (it->second.munmap_and_close_file()) {
				the_map.erase(it);
			}
		} else {
			throw mmap_allocator_exception("File "+fname+" not found in pool");
		}
	}

	mmap_file_pool the_pool; /* TODO: move to app object */
}
