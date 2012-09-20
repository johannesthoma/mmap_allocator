#include <memory>
#include <stdio.h>

namespace mmap_allocator_namespace
{
	template <typename T> 
	class mmap_allocator: public std::allocator<T>
	{
public:
		typedef size_t size_type; // TODO: use the inherited types
		typedef T* pointer;
		typedef const T* const_pointer;

		pointer allocate(size_type n, std::allocator<void>::const_pointer hint=0)
		{
			fprintf(stderr, "Alloc %d bytes.\n", n);
			return std::allocator<T>::allocate(n, hint);
		}

		mmap_allocator() throw(): std::allocator<T>() { fprintf(stderr, "Hallo allokator!\n"); }
		mmap_allocator(const mmap_allocator &a) throw(): std::allocator<T>(a) { }
//		~mmap_allocator() throw(): ~std::allocator<T>() { }
	};
}
