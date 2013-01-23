#ifndef _MMAP_EXCEPTION
#define _MMAP_EXCEPTION

#include "mmap_access_mode.h"
#include <stdexcept>

namespace mmap_allocator_namespace
{
	class mmap_allocator_exception: public std::runtime_error {
public:
		mmap_allocator_exception(const char *msg_param) throw(): 
			std::runtime_error(msg_param)
		{
			if (get_verbosity() > 0) {
				fprintf(stderr, "Throwing exception %s\n", msg_param);
			}
		}

		mmap_allocator_exception(std::string msg_param) throw(): 
			std::runtime_error(msg_param)
		{
			if (get_verbosity() > 0) {
				fprintf(stderr, "Throwing exception %s\n", msg_param.c_str());
			}
		}

		virtual ~mmap_allocator_exception(void) throw()
		{
		}
private:
	};
}

#endif
