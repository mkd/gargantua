#include "crash_recovery.h"

#ifndef _WIN32
thread_local bool in_syzygy_probe = false;
thread_local sigjmp_buf syzygy_jmp_buf;
#endif
