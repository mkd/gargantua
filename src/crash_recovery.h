#ifndef CRASH_RECOVERY_H
#define CRASH_RECOVERY_H

#ifndef _WIN32
#include <csetjmp>

extern thread_local bool in_syzygy_probe;
extern thread_local sigjmp_buf syzygy_jmp_buf;
#endif

#endif
