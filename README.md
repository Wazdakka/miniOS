# miniOS — OS Simulator in C

This C program is designed to take the place of PintOS for our Operating Systems preview.

## miniOS Conceptual Structure

miniOS will work differently from PintOS, which runs in a hardware emulator, allowing it to emulate the
differences between the hardware-based user and kernel modes.

For miniOS, we will be building all the pieces into one C program, and running at the user level. However,
internally the program is structured in layers that closely match real OSs - so for practical purposes
it works much the same as PintOS or Linux.

### Avoiding The Use of C Standard Library and Linux Systems Calls

Since miniOS is a user-level C program, we are actually *able* to call the C Standard Library, and make
system calls to the Linux kernel routines. This is true at both our virtual user code level and our
virtual kernel code level.

However, this will usually be something we want to avoid, partly to make more realistic exercises, but
more specifically because you won't have this option for the 439 classwork where you'll be using PintOS.
PintOS doesn't include the C Standard Library, even at the user level; you have to stick with PintOS
system calls.

This is a pain, because the C Standard Library provides a lot of utilities *other* than making Linux
system calls: string operations, number conversions, math, etc.

This is illustrated in some of the sample code in the project. There are places where is seems like it
should use a standard library routine like 'printf()', but instead it's more complicated. This is
because it has to use more basic calls provided by 'stdio'.

So, we should avoid stdlib calls except for:
 - Where it's really necessary in Kernel code (due to our lack of a hardware emulator), or
 - utility functions where it's not worth re-implementing ourselves.


## Project layout

This repository diagram shows the project-level structure. For a more conceptual diagram of how
these pieces fit into the general OS concepts that we're learning, see the "Intro to OS Anatomy"
document from our first session:
(https://docs.google.com/presentation/d/14WaNyTnM9XThpTDHDdYrWaFVzLOmr4zTdFGHZ4TwNiE/edit?usp=sharing)

miniOS/
├── include/
│   ├── syscall.h          # syscall numbers & gateway declaration (shared)
│   └── kernel.h           # kernel-internal types & declarations
│
├── kernel/                ### OUR KERNEL PROJECTS WILL BE HERE ###
│   ├── kernel_core.c      # THIS CONTAINS SYSCALLS THAT WE CREATE
│   ├── syscall_gateway.c  # the single kernel entry point (should be no need to edit this)
│   └── syscall_handler.c  # dispatcher + per-syscall handlers
│
├── user/                  ### USER-LEVEL CODE GOES HERE ###
│   ├── syscall_wrappers.h # library header
│   ├── syscall_wrappers.c # thin wrappers around syscall() (corollary of C std library)
│   ├── user_program.c     # example user program (demo)
│   └── shell.c            # minimal interactive shell to extend
│
└── Makefile



## Quick start

make              # compile everything
make run-demo     # run the user_program demo
make run-shell    # launch the interactive shell (type 'help' for available commands)
make clean        # remove build artefacts


## How to add a system call

### Step 1 — Add a new enum to `syscall_num_t` in `include/syscall.h`

### Step 2 — `kernel/syscall_handler.c`:
 - Add a case to `kernel_handle_syscall()`, with a new handler function name
 - Implement the new handler (in this file, or a new `kernel/fs.c`)

### Step 3 — Create a new wrapper for user space in `user/syscall_wrappers.h` and `.c`

### Step 4 — Use it in a user program


## Kernel logging

The kernel writes diagnostic output to **stderr** via `kprintf()`.
User output goes to **stdout** via `lib_write()`.
This means you can redirect them independently:

$ ./build/demo 2>/dev/null   # suppress kernel noise
$ ./build/demo 2>kernel.log  # capture kernel log separately
