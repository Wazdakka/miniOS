/*
 * user/syscall_wrappers.h  —  miniOS user-space library
 *
 * Thin, readable wrappers around the raw syscall() gateway.
 * User programs #include this instead of calling syscall() directly,
 * just as real programs use libc rather than raw int 0x80.
 *
 * Students: when you add a new SYS_* constant, add a matching
 * wrapper function here (declaration) and in syscall_wrappers.c
 * (definition).
 */

#ifndef MINIOS_SYSCALL_WRAPPERS_H
#define MINIOS_SYSCALL_WRAPPERS_H

#include <stddef.h>
#include <stdint.h>
#include "../include/syscall.h"

/* ------------------------------------------------------------------ *
 *  I/O                                                                *
 * ------------------------------------------------------------------ */

/* Write len bytes from buf to file descriptor fd.
 * Returns number of bytes written, or negative on error. */
syscall_result_t user_write(int fd, const char *buf, size_t len);

/* Convenience: write a NUL-terminated string to stdout. */
syscall_result_t user_puts(const char *s);

/* Read up to len-1 bytes from fd into buf (NUL-terminated).
 * Returns number of bytes read, or negative on error. */
syscall_result_t user_read(int fd, char *buf, size_t len);

/* ------------------------------------------------------------------ *
 *  Process control                                                    *
 * ------------------------------------------------------------------ */

/* Terminate the current process with the given exit status. */
void user_exit(int status);

/* Return this process's PID. */
int user_getpid(void);

/* Sleep for ms milliseconds. */
void user_sleep(unsigned int ms);

/* ------------------------------------------------------------------ *
 *  Memory                                                             *
 * ------------------------------------------------------------------ */

/* Allocate size bytes; returns pointer or NULL on failure. */
void *user_alloc(size_t size);

/* Free memory previously obtained from user_alloc. */
void  user_free(void *ptr);

/* ------------------------------------------------------------------ *
 * STUDENTS: declare your new wrapper functions below.
 *
 * Example:
 *   int  user_open (const char *path, int flags);
 *   int  user_close(int fd);
 *   int  user_fork (void);
 * ------------------------------------------------------------------ */

#endif /* MINIOS_SYSCALL_WRAPPERS_H */
