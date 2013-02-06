#ifndef _MMAP_ALLOCATOR_H
#define _MMAP_ALLOCATOR_H

#include <memory>
#include <string>
#include <stdio.h>
#include <vector>
#include "mmap_access_mode.h"
#include "mmap_exception.h"
#include "mmap_file_pool.h"

namespace mmap_allocator_namespace
{
	template <typename T, typename A>
        class mmappable_vector;

	template <typename T> 
	class mmap_allocator: public std::allocator<T>
	{
public:
		typedef size_t size_type;
		typedef off_t offset_type;
		typedef T* pointer;
		typedef const T* const_pointer;

		template<typename _Tp1>
	        struct rebind
        	{ 
			typedef mmap_allocator<_Tp1> other; 
		};

		pointer allocate(size_type n, const void *hint=0)
		{
			void *the_pointer;
			if (get_verbosity() > 0) {
				fprintf(stderr, "Alloc %zd bytes.\n", n*sizeof(T));
			}
			if (access_mode == DEFAULT_STL_ALLOCATOR) {
				return std::allocator<T>::allocate(n, hint);
			} else {
				if (n == 0) {
					return NULL;
				}
				if (bypass_file_pool) {
					the_pointer = private_file.open_and_mmap_file(filename, access_mode, offset, n*sizeof(T), map_whole_file, allow_remap);
				} else {
					the_pointer = the_pool.mmap_file(filename, access_mode, offset, n*sizeof(T), map_whole_file, allow_remap);
				}
				if (the_pointer == NULL) {
					throw(mmap_allocator_exception("Couldn't mmap file, mmap_file returned NULL"));
				}
				if (get_verbosity() > 0) {
					fprintf(stderr, "pointer = %p\n", the_pointer);
				}
				
				return (pointer)the_pointer;
			}
		}

		void deallocate(pointer p, size_type n)
		{
			if (get_verbosity() > 0) {
				fprintf(stderr, "Dealloc %zd bytes (%p).\n", n*sizeof(T), p);
			}
			if (access_mode == DEFAULT_STL_ALLOCATOR) {
				std::allocator<T>::deallocate(p, n);
			} else {
				if (n == 0) {
					return;
				}
				if (bypass_file_pool) {
					private_file.munmap_and_close_file();
				} else {
					if (!keep_forever) {
						the_pool.munmap_file(filename, access_mode, offset, n*sizeof(T));
					}
				}
			}
		}

		mmap_allocator() throw(): 
			std::allocator<T>(),
			filename(""),
			offset(0),
			access_mode(DEFAULT_STL_ALLOCATOR),
			map_whole_file(false),
			allow_remap(false),
			bypass_file_pool(false),
			private_file(),
			keep_forever(false)
		{ }

		mmap_allocator(const std::allocator<T> &a) throw():
			std::allocator<T>(a),
			filename(""),
			offset(0),
			access_mode(DEFAULT_STL_ALLOCATOR),
			map_whole_file(false),
			allow_remap(false),
			bypass_file_pool(false),
			private_file(),
			keep_forever(false)
		{ }

		mmap_allocator(const mmap_allocator &a) throw():
			std::allocator<T>(a),
			filename(a.filename),
			offset(a.offset),
			access_mode(a.access_mode),
			map_whole_file(a.map_whole_file),
			allow_remap(a.allow_remap),
			bypass_file_pool(a.bypass_file_pool),
			private_file(a.private_file),
			keep_forever(a.keep_forever)
		{ }
		mmap_allocator(const std::string filename_param, enum access_mode access_mode_param = READ_ONLY, offset_type offset_param = 0, int flags = 0) throw():
			std::allocator<T>(),
			filename(filename_param),
			offset(offset_param),
			access_mode(access_mode_param),
			map_whole_file((flags & MAP_WHOLE_FILE) != 0),
			allow_remap((flags & ALLOW_REMAP) != 0),
			bypass_file_pool((flags & BYPASS_FILE_POOL) != 0),
			private_file(),
			keep_forever((flags & KEEP_FOREVER) != 0)
		{
		}
			
		~mmap_allocator() throw() { }

private:
		friend class mmappable_vector<T, mmap_allocator<T> >;

		std::string filename;
		offset_type offset;
		enum access_mode access_mode;
		bool map_whole_file;
		bool allow_remap;
		bool bypass_file_pool;
		mmapped_file private_file;  /* used if bypass is set */
		bool keep_forever;
	};
}

#endif /* _MMAP_ALLOCATOR_H */
