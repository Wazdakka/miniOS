# miniOS — A Teaching OS Simulator in C

miniOS is a structured C project that teaches operating-systems concepts
(system calls, kernel/user separation, process control, memory management)
without requiring a PintOS or QEMU environment.

---

## Project layout

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
make run-shell    # launch the interactive shell
make clean        # remove build artefacts

---

## The kernel–user boundary

User code crosses into the kernel through **one single function**.

In the real OS this would happen inside of the the C Standard Library.
User code would call a library function (e.g.: malloc()), and the library
code would make the assembly-level syscall() to activate the hardware
trap into the kernel.

For our emulator, the C Standard Library will be emulated by the
syscall_wrappers. These are thin wrappers that are called by the user
programs, and internally issue a call to:

syscall_result_t syscall(syscall_num_t num,
                         uintptr_t a0, uintptr_t a1,
                         uintptr_t a2, uintptr_t a3);

---

## How to add a system call (the full walk-through)

Adding `SYS_OPEN` as an example:

### Step 1 — Name and number it (`include/syscall.h`)

```c
typedef enum {
    ...
    SYS_OPEN  = 8,   /* open(path, flags) → fd */
    SYS_CLOSE = 9,   /* close(fd)          → 0 or error */
    ...
    SYS_MAX
} syscall_num_t;
```

### Step 2 — Handle it in the kernel (`kernel/syscall_handler.c`)

Add a case to `kernel_handle_syscall`:

```c
case SYS_OPEN:  return handle_open(a0, a1);
```

Then implement the handler (in this file, or a new `kernel/fs.c`):

```c
static syscall_result_t handle_open(uintptr_t path, uintptr_t flags)
{
    const char *p = (const char *)path;
    /* ... open the file, return an fd or error code ... */
    return MINIOS_ENOSYS;   /* stub — replace this */
}
```

### Step 3 — Wrap it for user space (`user/syscall_wrappers.h` and `.c`)

Header:
```c
int user_open(const char *path, int flags);
```

Implementation:
```c
int user_open(const char *path, int flags)
{
    return (int)syscall(SYS_OPEN,
                        (uintptr_t)path,
                        (uintptr_t)flags,
                        0, 0);
}
```

### Step 4 — Use it in a user program

```c
int fd = user_open("hello.txt", 0);
if (fd < 0) { user_puts("open failed\n"); user_exit(1); }
```

---

## Return-value convention

| Value     | Meaning                          |
|-----------|----------------------------------|
| ≥ 0       | success (value depends on call)  |
| `MINIOS_EBADF`  (-1) | bad file descriptor     |
| `MINIOS_ENOMEM` (-2) | out of memory           |
| `MINIOS_EINVAL` (-3) | invalid argument        |
| `MINIOS_ENOSYS` (-4) | syscall not implemented |
| `MINIOS_EPERM`  (-5) | permission denied       |

---

## Extension ideas for students

### Kernel side
- **Filesystem**: add `SYS_OPEN`, `SYS_CLOSE`, `SYS_READ`/`SYS_WRITE` on
  file descriptors, backed by an in-memory or host filesystem.
- **Scheduler**: add a `process_t` table; implement `SYS_FORK`,
  `SYS_WAIT`, `SYS_KILL`; add a round-robin scheduler.
- **Virtual memory**: track allocations per process; implement
  `SYS_MMAP`/`SYS_MUNMAP`; detect double-free.
- **Signals**: add `SYS_SIGNAL` and `SYS_RAISE`.

### User side
- **Shell**: extend `user/shell.c` with new built-in commands; add pipes.
- **Processes**: write multiple "programs" in `user/` and have the kernel
  schedule between them.
- **Stress tests**: write a user program that hammers allocation/free in
  loops to test your kernel's memory management.

---

## Kernel logging

The kernel writes diagnostic output to **stderr** via `kprintf()`.
User output goes to **stdout** via `user_write()`.
This means you can redirect them independently:

```bash
./build/demo 2>/dev/null   # suppress kernel noise
./build/demo 2>kernel.log  # capture kernel log separately
```

---

## Build flags

The project compiles with `-Wall -Wextra -Wpedantic`.  Warnings are
treated seriously — keep the build clean.  The `-g` flag enables GDB
debugging; `gdb ./build/demo` just works.
