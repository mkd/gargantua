#ifndef CRASH_RECOVERY_H
#define CRASH_RECOVERY_H

#include <csetjmp>

extern thread_local bool in_syzygy_probe;
extern thread_local sigjmp_buf syzygy_jmp_buf;

#endif
