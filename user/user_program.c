/*
 * user/user_program.c  —  miniOS
 *
 * A sample user-space program.  This is the kind of code students
 * will write to exercise the kernel they are building.
 *
 * It intentionally uses only the wrapper functions from
 * syscall_wrappers.h — never raw syscall() calls, never stdio.
 *
 * Run via:  make run-demo
 */

#include <string.h>
#include "syscall_wrappers.h"

/* Forward declarations for our "processes" */
static void process_greeting(void);
static void process_memory(void);
static void process_io(void);

int main(void)
{
    user_puts("=== miniOS user-space demo ===\n\n");

    process_greeting();
    process_memory();
    process_io();

    user_puts("\nAll done. Exiting.\n");
    user_exit(0);
}

/* ------------------------------------------------------------------ *
 *  Demo 1: basic I/O and PID                                         *
 * ------------------------------------------------------------------ */
static void process_greeting(void)
{
    int pid = user_getpid();

    char buf[64];
    /* Build message manually — no sprintf in "user space" */
    const char *msg1 = "Hello from user space! My PID is ";
    user_write(1, msg1, strlen(msg1));

    /* tiny itoa for the demo */
    int n = pid;
    int i = 0;
    if (n == 0) { buf[i++] = '0'; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    buf[i] = '\0';
    /* reverse */
    for (int l = 0, r = i - 1; l < r; l++, r--) {
        char t = buf[l]; buf[l] = buf[r]; buf[r] = t;
    }
    user_puts(buf);
    user_puts("\n");
}

/* ------------------------------------------------------------------ *
 *  Demo 2: dynamic memory via kernel alloc/free                      *
 * ------------------------------------------------------------------ */
static void process_memory(void)
{
    user_puts("Requesting 128 bytes from kernel... ");
    char *block = (char *)user_alloc(128);
    if (!block) {
        user_puts("FAILED (returned NULL)\n");
        return;
    }
    user_puts("got it.\n");

    /* Write and read back */
    const char *payload = "data stored in kernel-allocated memory";
    size_t len = strlen(payload);
    for (size_t i = 0; i < len; i++) block[i] = payload[i];
    block[len] = '\0';

    user_puts("Contents: ");
    user_puts(block);
    user_puts("\n");

    user_puts("Freeing block... ");
    user_free(block);
    user_puts("done.\n");
}

/* ------------------------------------------------------------------ *
 *  Demo 3: reading from stdin via the kernel                         *
 * ------------------------------------------------------------------ */
static void process_io(void)
{
    char buf[128];
    user_puts("Enter something (or press Enter to skip): ");
    syscall_result_t n = user_read(0, buf, sizeof(buf));
    if (n > 0) {
        user_puts("Kernel echoes: ");
        user_write(1, buf, (size_t)n);
    }
}
