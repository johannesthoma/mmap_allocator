#include <memory>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace mmap_allocator_namespace
{
	class mmap_allocator_exception: public std::exception {
		std::string msg; 

		mmap_allocator_exception() throw(): 
			std::exception(),
			msg("Unknown reason")
		{
		}

		mmap_allocator_exception(const char *msg_param) throw(): 
			std::exception(),
			msg(msg_param)
		{
		}

		virtual ~mmap_allocator_exception(void) throw()
		{
		}

		const char *message(void)
		{
			return msg.c_str();
		}
	};

	template <typename T> 
	class mmap_allocator: public std::allocator<T>
	{
public:
		typedef size_t size_type; // TODO: use the inherited types
		typedef T* pointer;
		typedef const T* const_pointer;

		template<typename _Tp1>
	        struct rebind
        	{ 
			typedef mmap_allocator<_Tp1> other; 
		};

		pointer allocate(size_type n, const void *hint=0)
		{
			fprintf(stderr, "Alloc %d bytes.\n", n);
			return std::allocator<T>::allocate(n, hint);
		}

		void deallocate(pointer p, size_type n)
		{
			fprintf(stderr, "Dealloc %d bytes (%p).\n", n, p);
			return std::allocator<T>::deallocate(p, n);
		}

		mmap_allocator() throw(): std::allocator<T>() { }
		mmap_allocator(const mmap_allocator &a) throw():
			std::allocator<T>(a),
			filename(a.filename),
			offset(a.offset),
                        fd(a.fd)
		{ }
		mmap_allocator(const std::string filename_param, off_t offset_param) throw():
			std::allocator<T>(),
			filename(filename_param),
			offset(offset_param)
		{
			open_and_mmap_file();
		}
			
		~mmap_allocator() throw() { }

private:
		std::string filename;
		off_t offset;
		int fd;

		void open_and_mmap_file(void) throw()
		{
			fd = open(filename.c_str(), O_RDONLY);
			if (fd < 0) {
				throw mmap_allocator_exception("No such file or directory");
			}
		}
	};
}
