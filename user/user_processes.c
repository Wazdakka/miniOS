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

    lib_spawn(processBody, "lapras");
    lib_spawn(processBody, "charizard");
    lib_process();

    lib_puts("\nAll done. Exiting.\n");
    lib_exit(0);
}

void *processBody(void *pokemonName) {
    for (int i = 0; i < 1000; i++) {
        lib_puts("I choose you, ");
        lib_puts((char*)pokemonName);
        lib_puts("!\n");
    }
    return NULL;
}