/*
 * user/syscall_wrappers.c  —  miniOS user-space library
 *
 * Implements the wrappers declared in syscall_wrappers.h.
 * Each function is intentionally short — its only job is to marshal
 * arguments into the four uintptr_t slots and call syscall().
 */

#include <string.h>
#include <stddef.h>
#include "syscall_wrappers.h"

/* ------------------------------------------------------------------ *
 *  I/O                                                                *
 * ------------------------------------------------------------------ */

syscall_result_t user_write(int fd, const char *buf, size_t len)
{
    return syscall(SYS_WRITE,
                   (uintptr_t)fd,
                   (uintptr_t)buf,
                   (uintptr_t)len,
                   0);
}

syscall_result_t user_puts(const char *s)
{
    return user_write(1, s, strlen(s));
}

syscall_result_t user_read(int fd, char *buf, size_t len)
{
    return syscall(SYS_READ,
                   (uintptr_t)fd,
                   (uintptr_t)buf,
                   (uintptr_t)len,
                   0);
}

/* ------------------------------------------------------------------ *
 *  Process control                                                    *
 * ------------------------------------------------------------------ */

void user_exit(int status)
{
    syscall(SYS_EXIT, (uintptr_t)status, 0, 0, 0);
    /* unreachable */
}

int user_getpid(void)
{
    return (int)syscall(SYS_GETPID, 0, 0, 0, 0);
}

void user_sleep(unsigned int ms)
{
    syscall(SYS_SLEEP, (uintptr_t)ms, 0, 0, 0);
}

/* ------------------------------------------------------------------ *
 *  Memory                                                             *
 * ------------------------------------------------------------------ */

void *user_alloc(size_t size)
{
    syscall_result_t r = syscall(SYS_ALLOC, (uintptr_t)size, 0, 0, 0);
    return (r < 0) ? NULL : (void *)(uintptr_t)r;
}

void user_free(void *ptr)
{
    syscall(SYS_FREE, (uintptr_t)ptr, 0, 0, 0);
}

/* ------------------------------------------------------------------ *
 * STUDENTS: implement your new wrapper functions below.
 *
 * Pattern to follow:
 *
 *   return_type user_foo(arg_type arg, ...)
 *   {
 *       return syscall(SYS_FOO,
 *                      (uintptr_t)arg,
 *                      ...
 *                      0);
 *   }
 * ------------------------------------------------------------------ */
