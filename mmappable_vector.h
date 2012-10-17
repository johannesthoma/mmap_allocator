#ifndef _MMAPPABLE_VECTOR_H
#define _MMAPPABLE_VECTOR_H

#include <memory>
#include <string>
#include <stdio.h>
#include <vector>
#include "mmap_allocator.h"

namespace mmap_allocator_namespace {
	template <typename T, typename A = mmap_allocator<T> > 
	class mmappable_vector: public std::vector<T, A> {
public:
/* TODO: are these necessary here? */
		typedef typename std::vector<T, A>::const_iterator const_iterator;
		typedef typename std::vector<T, A>::iterator iterator;
		typedef T value_type;
		typedef A allocator_type;

		mmappable_vector():
			std::vector<T,A>()
		{
		}

		mmappable_vector(const mmappable_vector<T, A> &other):
			std::vector<T,A>(other)
		{
		}

		explicit mmappable_vector(size_t n):
			std::vector<T,A>()
		{
			mmap_file(n);
		}

		explicit mmappable_vector(A alloc):
			std::vector<T,A>(alloc)
		{
		}

		mmappable_vector(iterator from, iterator to):
			std::vector<T,A>(from, to)
		{
		}
		
/*
		mmappable_vector(std::vector<T,std::allocator<T> >::iterator from, std::vector<T,std::allocator<T> >::iterator to):
			std::vector<T,A>(from, to)
		{
		}
*/

		mmappable_vector(int n, T val, A alloc):
			std::vector<T,A>(n, val, alloc)
		{
		}

		mmappable_vector(int n, T val):
			std::vector<T,A>(n, val)
		{
		}

		mmappable_vector(std::vector<T,std::allocator<T> > v):
			std::vector<T,std::allocator<T> >(v)
		{
		}

/* Use this only when the allocator is already initialized. */
		void mmap_file(size_t n)
		{
			std::vector<T,A>::reserve(n);
			_M_set_finish(n);
		}

		void mmap_file(std::string filename, enum access_mode access_mode, const off_t offset, const size_t n, int flags = 0)
		{
			if (std::vector<T,A>::size() > 0) {
				throw mmap_allocator_exception("Remapping currently not implemented.");
			}
#ifdef __GNUC__
#if __GNUC__ == 4
			A *the_allocator = &std::vector<T,A>::_M_get_Tp_allocator();
#elif __GNUC__ == 3
			A *the_allocator = static_cast<A*>(&(this->std::vector<T,A>::_M_impl));
#else
#error "GNU C++ not version 3 or 4, please implement me"
#endif		
#else
#error "Not GNU C++, please either implement me or use GCC"
#endif		
			the_allocator->filename = filename;
			the_allocator->offset = offset;
			the_allocator->access_mode = access_mode;
			the_allocator->map_whole_file = (flags & MAP_WHOLE_FILE) != 0;
			the_allocator->allow_remap = (flags & ALLOW_REMAP) != 0;
			the_allocator->bypass_file_pool = (flags & BYPASS_FILE_POOL) != 0;

			mmap_file(n);
		}

		void munmap_file(void)
		{
			size_t n = std::vector<T,A>::size();
#ifdef __GNUC__
			std::vector<T,A>::_M_deallocate(std::vector<T,A>::_M_impl._M_start, n);
			std::vector<T,A>::_M_impl._M_start = 0;
			std::vector<T,A>::_M_impl._M_finish = 0;
			std::vector<T,A>::_M_impl._M_end_of_storage = 0;
#else
#error "Not GNU C++, please either implement me or use GCC"
#endif
		}

private:
		void _M_set_finish(size_t n)
		{
#ifdef __GNUC__
			std::vector<T,A>::_M_impl._M_finish = std::vector<T,A>::_M_impl._M_start + n;
#else
#error "Not GNU C++, please either implement me or use GCC"
#endif		
		}
	};

	template <typename T> 
	std::vector<T> to_std_vector(const mmappable_vector<T> &v)
	{
		return std::vector<T>(v.begin(), v.end());
	}

	template <typename T> 
	mmappable_vector<T> to_mmappable_vector(const std::vector<T> &v)
	{
		// return mmappable_vector<T>(v.begin(), v.end());
		return (mmappable_vector<T>)v;
	}
}

#endif /* MMAPPABLE_VECTOR_H */
