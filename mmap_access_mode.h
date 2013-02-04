#ifndef _MMAP_ACCESS_MODE
#define _MMAP_ACCESS_MODE

#include <stdio.h>

#define ALIGN_TO_PAGE(x) ((x) & ~(getpagesize() - 1))
#define UPPER_ALIGN_TO_PAGE(x) ALIGN_TO_PAGE((x)+(getpagesize()-1))
#define OFFSET_INTO_PAGE(x) ((x) & (getpagesize() - 1))

namespace mmap_allocator_namespace
{
	enum access_mode {
		DEFAULT_STL_ALLOCATOR, /* Default STL allocator (malloc based). Reason is to have containers that do both and are compatible */
		READ_ONLY,  /* Readonly modus. Segfaults when vector content is written to */
		READ_WRITE_PRIVATE, /* Read/write access, writes are not propagated to disk */
		READ_WRITE_SHARED  /* Read/write access, writes are propagated to disk (file is modified) */
	};

	enum allocator_flags {
		MAP_WHOLE_FILE = 1,
		ALLOW_REMAP = 2,
		BYPASS_FILE_POOL = 4,
		KEEP_FOREVER = 8
	};

	void set_verbosity(int v);
	int get_verbosity(void);
}

#endif
