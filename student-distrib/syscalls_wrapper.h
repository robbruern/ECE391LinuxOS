/* syscalls_wrapper.h - Assembly linkage wrapper for system call handler.
 * vim:ts=4 noexpandtab
 */

#ifndef _SYSCALLSWRAPPER_H
#define _SYSCALLSWRAPPER_H

#include "types.h"
#include "syscalls.h"

extern int32_t system_call();

#endif
