#include <memory>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace mmap_allocator_namespace
{
	class mmap_allocator_exception: public std::exception {
public:
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
private:
		std::string msg; 
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
			open_and_mmap_file(n);
			fprintf(stderr, "mem-area = %p\n", memory_area);
			return (pointer)memory_area;
		}

		void deallocate(pointer p, size_type n)
		{
			fprintf(stderr, "Dealloc %d bytes (%p).\n", n, p);
			return std::allocator<T>::deallocate(p, n);
		}

		mmap_allocator() throw(): 
			std::allocator<T>(),
			filename(""),
			offset(0),
			fd(-1),
			memory_area(NULL)
		{ }
		mmap_allocator(const mmap_allocator &a) throw():
			std::allocator<T>(a),
			filename(a.filename),
			offset(a.offset),
                        fd(a.fd),
			memory_area(a.memory_area)
		{ }
		mmap_allocator(const std::string filename_param, off_t offset_param = 0) throw():
			std::allocator<T>(),
			filename(filename_param),
			offset(offset_param),
			fd(-1),
			memory_area(NULL)
		{
		}
			
		~mmap_allocator() throw() { }

private:
		std::string filename;
		off_t offset;
		int fd;
		void *memory_area;

		void open_and_mmap_file(size_t length)
		/* Must not be called from within constructors since they must not throw exceptions */
		{
			if (filename.c_str()[0] == '\0') {
				throw mmap_allocator_exception("mmap_allocator not correctly initialized: filename is empty.");
			}
			// fd = open(filename.c_str(), O_RDONLY);
			fd = open(filename.c_str(), O_RDWR);
			if (fd < 0) {
				throw mmap_allocator_exception("No such file or directory");
			}
//			memory_area = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, offset);
			memory_area = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
			if (memory_area == MAP_FAILED) {
				throw mmap_allocator_exception("Error in mmap");
			}
		}
	};
}
