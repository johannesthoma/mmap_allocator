#ifndef _MMAP_ACCESS_MODE
#define _MMAP_ACCESS_MODE

#include <stdio.h>

#define ALIGN_TO_PAGE(x) ((x) & ~(getpagesize() - 1))
#define UPPER_ALIGN_TO_PAGE(x) ALIGN_TO_PAGE((x)+(getpagesize()-1))
#define OFFSET_INTO_PAGE(x) ((x) & (getpagesize() - 1))

// #define MMAP_ALLOCATOR_DEBUG 1

namespace mmap_allocator_namespace
{
	enum access_mode {
		DEFAULT_STL_ALLOCATOR, /* Default STL allocator (malloc based). Reason is to have containers that do both and are compatible */
		READ_ONLY,  /* Readonly modus. Segfaults when vector content is written to */
		READ_WRITE_PRIVATE, /* Read/write access, writes are not propagated to disk */
		READ_WRITE_SHARED  /* Read/write access, writes are propagated to disk (file is modified) */
	};

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
#ifdef MMAP_ALLOCATOR_DEBUG
			fprintf(stderr, "Throwing exception %s\n", msg_param);
#endif
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
}

#endif
