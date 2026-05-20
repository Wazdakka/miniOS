/*
 * kernel/syscall_gateway.c  —  miniOS
 *
 * The single crossing point from user space into the kernel.
 *
 * On real hardware this would be a trap / software interrupt.
 * Here it is a C function call, but the abstraction is identical:
 *   - user code calls syscall() with a number and up to 4 arguments
 *   - the gateway validates the call number
 *   - the gateway forwards to kernel_handle_syscall()
 *   - the result (≥0 success, <0 error) comes back to the caller
 *
 * Students: you should NOT need to edit this file.
 *           Add new behaviour in kernel/syscall_handler.c.
 */

#include "../include/syscall.h"
#include "../include/kernel.h"

syscall_result_t syscall(syscall_num_t num,
                         uintptr_t    a0,
                         uintptr_t    a1,
                         uintptr_t    a2,
                         uintptr_t    a3)
{
    /* --- Range-check the call number before touching the kernel --- */
    if (num <= 0 || num >= SYS_MAX) {
        return MINIOS_ENOSYS;   /* unknown syscall */
    }

    /* --- Cross the boundary --- */
    return kernel_handle_syscall(num, a0, a1, a2, a3);
}
