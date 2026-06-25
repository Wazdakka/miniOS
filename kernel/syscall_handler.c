/*
 * kernel/syscall_handler.c  —  miniOS
 *
 * This is where the kernel actually services each system call.
 * 
 * This code contains the single kernel entry point - kernel_handle_syscall().
 * That function checks which system call has been requested and calls a specific
 * handler for that particular system call.
 *
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* usleep */
#include <time.h>      /* nanosleep fallback */
#include "../include/kernel.h"
#include "../include/syscall.h"
#include "../include/kernel_utils.h"

/* ------------------------------------------------------------------ *
 *  Forward declarations for internal helpers                         *
 * ------------------------------------------------------------------ */
static syscall_result_t handle_write (uintptr_t fd, uintptr_t buf,
                                      uintptr_t len);
static syscall_result_t handle_read  (uintptr_t fd, uintptr_t buf,
                                      uintptr_t len);
static syscall_result_t handle_spawn (uintptr_t func_ptr, uintptr_t arg_ptr);
static syscall_result_t handle_process();
static syscall_result_t handle_lockinit();
static syscall_result_t handle_lock();
static syscall_result_t handle_unlock();
static syscall_result_t handle_yield();
static syscall_result_t handle_done();
static syscall_result_t handle_exit  (uintptr_t status);
static syscall_result_t handle_getpid(void);
static syscall_result_t handle_sleep (uintptr_t ms);
static syscall_result_t handle_alloc (uintptr_t size);
static syscall_result_t handle_free  (uintptr_t ptr);

/* ------------------------------------------------------------------ *
 *  Simple kernel state                                               *
 * ------------------------------------------------------------------ */
int next_pid = 1;   /* process-ID counter */
int current_processes = 0;
atomic_flag lock = ATOMIC_FLAG_INIT;
process_t process_table[MAX_PROCESSES];
process_t* current_process_ptr = NULL;
pthread_mutex_t process_lock = PTHREAD_MUTEX_INITIALIZER;

/* ------------------------------------------------------------------ *
 *  Dispatcher — the heart of the kernel                              *
 * ------------------------------------------------------------------ */
syscall_result_t kernel_handle_syscall(syscall_num_t num,
                                       uintptr_t a0,
                                       uintptr_t a1,
                                       uintptr_t a2,
                                       uintptr_t a3)
{
    kprintf("[kernel] syscall %d  args=(%lu, %lu, %lu, %lu)\n",
            num, a0, a1, a2, a3);

    // When adding kernel functions, add a case here, and a new "handle_*()" function
    switch (num) {
        case SYS_WRITE:  return handle_write (a0, a1, a2);
        case SYS_READ:   return handle_read  (a0, a1, a2);
        case SYS_SPAWN:  return handle_spawn(a0, a1);
        case SYS_PROCESS:return handle_process();
        case SYS_LOCKINIT:return handle_lockinit();
        case SYS_LOCK:   return handle_lock();
        case SYS_UNLOCK: return handle_unlock();
        case SYS_YIELD:  return handle_yield();
        case SYS_DONE:   return handle_done();
        case SYS_EXIT:   return handle_exit  (a0);
        case SYS_GETPID: return handle_getpid();
        case SYS_SLEEP:  return handle_sleep (a0);
        case SYS_ALLOC:  return handle_alloc (a0);
        case SYS_FREE:   return handle_free  (a0);

        default:
            kprintf("[kernel] unknown syscall %d — returning ENOSYS\n", num);
            return MINIOS_ENOSYS;
    }
}

/* ------------------------------------------------------------------ *
 *  Handler implementations                                           *
 * ------------------------------------------------------------------ */

static syscall_result_t handle_write(uintptr_t fd,
                                     uintptr_t buf,
                                     uintptr_t len)
{
    const char *s = (const char *)buf;

    if (!s)                    return MINIOS_EINVAL;
    if (fd != 1 && fd != 2)   return MINIOS_EBADF;

    FILE *stream = (fd == 1) ? stdout : stderr;
    size_t written = fwrite(s, 1, (size_t)len, stream);
    return (syscall_result_t)written;
}

static syscall_result_t handle_read(uintptr_t fd,
                                    uintptr_t buf,
                                    uintptr_t len)
{
    char *s = (char *)buf;

    if (!s)    return MINIOS_EINVAL;
    if (fd != 0) return MINIOS_EBADF;

    if (!fgets(s, (int)len, stdin))
        return 0;
    return (syscall_result_t)strlen(s);
}

static syscall_result_t handle_spawn(uintptr_t func_ptr, uintptr_t arg_ptr) {
    // if (current_processes >= MAX_PROCESSES) { return MINIOS_EMAXPROCESSES; }
    // int i = current_processes++;
    // int pid = next_pid++;

    // process_table[i].pid = pid;
    // process_table[i].state = PROC_READY;
    // process_table[i].thread_func_ptr = (void *(*)(void *))func_ptr;
    // process_table[i].thread_arg_ptr = (void *)arg_ptr;
    return MINIOS_OK;
}

static syscall_result_t handle_process() {
    // for (int i = 0; i < current_processes; i++) {
    //     pthread_create(
    //     &process_table[i].thread, //receptacle for data about thread
    //     NULL,
    //     process_table[i].thread_func_ptr,
    //     process_table[i].thread_arg_ptr
    //     );
    //     process_table[i].state = PROC_RUNNING;
    // }

    // while (current_processes > 0) {
    //     pthread_join(process_table[current_processes - 1].thread, NULL);
    //     process_table[current_processes - 1].state = PROC_DONE;
    //     current_processes--;
    // }

    return MINIOS_OK;
}

static syscall_result_t handle_lockinit() { return 0; }
static syscall_result_t handle_lock() 
{ 
    while (atomic_flag_test_and_set(&lock)) { ; } //if unset, set it and return false; if set, simply return true
    return MINIOS_OK;
}
static syscall_result_t handle_unlock() 
{ 
    atomic_flag_clear(&lock);
    return MINIOS_OK;
}
static syscall_result_t handle_yield() { return 0; }
static syscall_result_t handle_done() { return 0; }


static syscall_result_t handle_exit(uintptr_t status)
{
    kprintf("[kernel] process exiting with status %lu\n", status);
    exit((int)status);
    return MINIOS_OK;   /* unreachable, but keeps the compiler happy */
}

static syscall_result_t handle_getpid(void)
{
    /* In a real kernel this would return the running process's PID.
     * Here we hand out incrementing IDs to illustrate the concept. */
    return current_process_ptr->pid;
}

static syscall_result_t handle_sleep(uintptr_t ms)
{
    struct timespec ts;
    ts.tv_sec  = (time_t)(ms / 1000);
    ts.tv_nsec = (long)((ms % 1000) * 1000000L);
    nanosleep(&ts, NULL);
    return MINIOS_OK;
}

static syscall_result_t handle_alloc(uintptr_t size)
{
    if (size == 0) return MINIOS_EINVAL;

    void *ptr = malloc((size_t)size);
    if (!ptr) return MINIOS_ENOMEM;

    kprintf("[kernel] alloc %lu bytes → %p\n", size, ptr);
    return (syscall_result_t)(uintptr_t)ptr;
}

static syscall_result_t handle_free(uintptr_t ptr)
{
    if (!ptr) return MINIOS_EINVAL;
    free((void *)ptr);
    kprintf("[kernel] free %p\n", (void *)ptr);
    return MINIOS_OK;
}
