#ifndef _MMAP_ALLOCATOR_H
#define _MMAP_ALLOCATOR_H

#include <memory>
#include <string>
#include <stdio.h>
#include <vector>
#include "mmap_access_mode.h"
#include "mmap_file_pool.h"

namespace mmap_allocator_namespace
{
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
#ifdef MMAP_ALLOCATOR_DEBUG
			fprintf(stderr, "Alloc %d bytes.\n", n*sizeof(T));
#endif
			if (access_mode == DEFAULT_STL_ALLOCATOR) {
				return std::allocator<T>::allocate(n, hint);
			} else {
				the_pointer = the_pool.mmap_file(filename, access_mode, offset, n*sizeof(T), map_whole_file, allow_remap);
				if (the_pointer == NULL) {
					throw(mmap_allocator_exception("Couldn't mmap file, mmap_file returned NULL"));
				}
#ifdef MMAP_ALLOCATOR_DEBUG
				fprintf(stderr, "pointer = %p\n", the_pointer);
#endif
				
				return (pointer)the_pointer;
			}
		}

		void deallocate(pointer p, size_type n)
		{
#ifdef MMAP_ALLOCATOR_DEBUG
			fprintf(stderr, "Dealloc %d bytes (%p).\n", n*sizeof(T), p);
#endif
			if (access_mode == DEFAULT_STL_ALLOCATOR) {
				std::allocator<T>::deallocate(p, n);
			} else {
				the_pool.munmap_file(filename, access_mode, offset, n*sizeof(T));
			}
		}

		mmap_allocator() throw(): 
			std::allocator<T>(),
			filename(""),
			offset(0),
			access_mode(DEFAULT_STL_ALLOCATOR),
			map_whole_file(false),
			allow_remap(false)
		{ }

		mmap_allocator(const std::allocator<T> &a) throw():
			std::allocator<T>(a),
			filename(""),
			offset(0),
			access_mode(DEFAULT_STL_ALLOCATOR),
			map_whole_file(false),
			allow_remap(false)
		{ }

		mmap_allocator(const mmap_allocator &a) throw():
			std::allocator<T>(a),
			filename(a.filename),
			offset(a.offset),
			access_mode(a.access_mode),
			map_whole_file(a.map_whole_file),
			allow_remap(a.allow_remap)
		{ }
		mmap_allocator(const std::string filename_param, enum access_mode access_mode_param = READ_ONLY, offset_type offset_param = 0, bool map_whole_file_param = false, bool allow_remap_param = false) throw():
			std::allocator<T>(),
			filename(filename_param),
			offset(offset_param),
			access_mode(access_mode_param),
			map_whole_file(map_whole_file_param),
			allow_remap(allow_remap_param)
		{
		}
			
		~mmap_allocator() throw() { }

private:
		std::string filename;
		offset_type offset;
		enum access_mode access_mode;
		bool map_whole_file;
		bool allow_remap;
	};
}

template <typename T> std::vector<T> to_std_vector(const std::vector <T, mmap_allocator_namespace::mmap_allocator<T> > &v)
{
	return std::vector<T, std::allocator<T> >(v.begin(), v.end());
}

template <typename T>
std::vector<T, mmap_allocator_namespace::mmap_allocator<T> > to_mmap_vector(std::vector<T, std::allocator<T> > &v)
{
	return std::vector<T, mmap_allocator_namespace::mmap_allocator<T> >(v.begin(), v.end());
}

#endif /* _MMAP_ALLOCATOR_H */
