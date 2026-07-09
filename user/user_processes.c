/*
 * user/user_program.c  —  miniOS
 *
 * A sample user-space program.  This is representative of
 * the code that runs in the user space and calls the
 * library (wrapper) functions to access miniOS functionality.
 *
 * Run via:  make run-demo
 */

#include <string.h>
#include "syscall_wrappers.h"

/* Forward declarations for our "processes" */
void *processBody(void *pokemonName);

int main(void)
{
    lib_puts("=== miniOS user-space demo ===\n\n");
    lib_lockinit();
    lib_spawn(processBody, "lapras");
    lib_spawn(processBody, "charizard");
    lib_spawn(processBody, "sudowudo");
    lib_process();

    lib_puts("\nAll done. Exiting.\n");
    lib_exit(0);
}

void *processBody(void *pokemonName) {
    for (int i = 0; i < 10; i++) {
        lib_yield();
        lib_lock();
        lib_puts("I choose you, ");
        lib_puts((char*)pokemonName);
        lib_puts("!\n");
        lib_unlock();
        lib_sleep(250);
    }
    lib_done();
    return NULL;
}