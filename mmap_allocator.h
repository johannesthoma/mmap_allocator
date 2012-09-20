#include <memory>
#include <stdio.h>

namespace mmap_allocator_namespace
{
	template <typename T> 
	class mmap_allocator: public std::allocator<T>
	{
		typedef size_t size_type;
		typedef T* pointer;
		typedef const T* const_pointer;

		pointer allocate(size_type n, const_pointer hint=0)
		{
			fprintf(stderr, "Alloc %d bytes.\n", n);
			return std::allocator<T>::allocate(n, hint);
		}
	};
}
