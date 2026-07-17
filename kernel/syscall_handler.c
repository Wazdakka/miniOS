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
bool is_kernel_initialized = false;
int next_pid = 1;   /* process-ID counter */
int current_processes = 0;
bool lock = false;
int lock_owner_pid = -1;
process_t process_table[MAX_PROCESSES];
process_t* current_process_ptrs[NUM_CORES] = { NULL };
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

    pthread_mutex_lock(&process_lock); 
    if (!is_kernel_initialized) {
        kernel_init();
    }
    pthread_mutex_unlock(&process_lock); 

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
    pthread_mutex_lock(&process_lock); //modifying kernel state, so lock
    int empty_index = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == 0) {
            empty_index = i;
            break;
        }
    }
    if (empty_index == -1) { return MINIOS_EMAXPROCESSES; }
    process_table[empty_index].pid = next_pid++;

    process_table[empty_index].state = PROC_READY;
    process_table[empty_index].run_flag = false;
    process_table[empty_index].condition = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    pthread_create(
        &process_table[empty_index].thread, //receptacle for data about thread
        NULL,
        (void *(*)(void *))func_ptr,
        (void *)arg_ptr
    );
    current_processes++;
    
    pthread_mutex_unlock(&process_lock); 
    return process_table[empty_index].pid;
}

static syscall_result_t handle_process() {
    pthread_exit(NULL); //kill main thread without killing any other threads
    return MINIOS_OK;
}

static syscall_result_t handle_lockinit() { return 0; }
static syscall_result_t handle_lock() 
{ 
    pthread_mutex_lock(&process_lock); //modifying kernel state, so lock
    process_t *self_process = find_process_self();
    if (lock && self_process->pid != lock_owner_pid) { //locking twice in a row only half-handled, broken by multiple unlocks
        swap_process_out(find_process_self(), PROC_WAIT_LOCK);
    }
    lock = true; //we've been reawoken after the lock was released, so grab it
    lock_owner_pid = self_process->pid;
    pthread_mutex_unlock(&process_lock); 
    return MINIOS_OK;
}
static syscall_result_t handle_unlock() 
{ 
    pthread_mutex_lock(&process_lock); //modifying kernel state, so lock
    process_t *self_process = find_process_self();
    if (lock && self_process->pid == lock_owner_pid) {
        lock = false;
        lock_owner_pid = -1;

        process_t *waiting_process = find_process_by_state(PROC_WAIT_LOCK); //wake up a process waiting for the lock, if any
        int idle_core = find_idle_core();
        if (waiting_process) {
            if (idle_core != -1) { //a process left an empty spot while waiting for lock, so fill it again
                swap_process_in(waiting_process, idle_core);
            } else { //a process was waiting for the lock -- swap to them
                swap_process_in(waiting_process, find_core_for_process(self_process));
                swap_process_out(self_process, PROC_READY);
            }
        }
    }
    pthread_mutex_unlock(&process_lock);
    return MINIOS_OK;
}
static syscall_result_t handle_yield() { 
    pthread_mutex_lock(&process_lock);
    process_t *self_process = find_process_self();

    if (self_process->state == PROC_RUNNING) {
        bool times_up = is_timeslice_expired(&self_process->slice_expire_time);
        if (times_up) {
        if (swap_in_ready_process(find_core_for_process(self_process)) != NULL) {
            //swap ourselves out iff 1. timeslice is up, and 2. another thread is ready to run
            swap_process_out(self_process, PROC_READY);
            current_process_ptrs[find_core_for_process(self_process)] = NULL;
        }
    }
    } else { //we aren't running, swap in if able
        int open_core = find_idle_core();
        if (open_core != -1) {
            swap_process_in(self_process, open_core); 
        } else {
            swap_process_out(self_process, PROC_READY);
        }
    }

    pthread_mutex_unlock(&process_lock);
    return 0; 
}
static syscall_result_t handle_done() { 
    
    process_t *self_process = find_process_self();
    int self_core = find_core_for_process(self_process);

    handle_unlock(); //drop lock if we have it
    pthread_mutex_lock(&process_lock);
    self_process->pid = 0;
    if (swap_in_ready_process(self_core) == NULL) {
        current_process_ptrs[self_core] = NULL;
    }
    current_processes--;
    pthread_mutex_unlock(&process_lock);
    return 0;
}


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
    return find_process_self()->pid;
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
