# miniOS Makefile
#
# Targets:
#   make              — build everything
#   make run-demo     — build & run the user_program demo
#   make shell        — build the interactive shell
#   make run-shell    — build & run the interactive shell
#   make clean        — remove build artefacts

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g \
           -I./include

BUILD   := build

# ------------------------------------------------------------------ #
#  Source lists                                                        #
# ------------------------------------------------------------------ #

KERNEL_SRC := \
    kernel/kernel_core.c    \
    kernel/syscall_gateway.c \
    kernel/syscall_handler.c

USER_SRC := \
    user/syscall_wrappers.c

DEMO_SRC  := user/user_program.c
SHELL_SRC := user/shell.c

# ------------------------------------------------------------------ #
#  Object lists                                                        #
# ------------------------------------------------------------------ #

KERNEL_OBJ := $(patsubst %.c, $(BUILD)/%.o, $(KERNEL_SRC))
USER_OBJ   := $(patsubst %.c, $(BUILD)/%.o, $(USER_SRC))
DEMO_OBJ   := $(patsubst %.c, $(BUILD)/%.o, $(DEMO_SRC))
SHELL_OBJ  := $(patsubst %.c, $(BUILD)/%.o, $(SHELL_SRC))

ALL_OBJ    := $(KERNEL_OBJ) $(USER_OBJ) $(DEMO_OBJ) $(SHELL_OBJ)

# ------------------------------------------------------------------ #
#  Top-level targets                                                   #
# ------------------------------------------------------------------ #

.PHONY: all run-demo shell run-shell clean

all: $(BUILD)/demo $(BUILD)/shell
	@echo ""
	@echo "  Built:"
	@echo "    ./$(BUILD)/demo   — run the user-program demo"
	@echo "    ./$(BUILD)/shell  — launch the interactive shell"

run-demo: $(BUILD)/demo
	@echo "--- running demo ---"
	@echo "" | ./$(BUILD)/demo

run-shell: $(BUILD)/shell
	@echo "--- launching shell ---"
	./$(BUILD)/shell

# ------------------------------------------------------------------ #
#  Linking                                                             #
# ------------------------------------------------------------------ #

$(BUILD)/demo: $(KERNEL_OBJ) $(USER_OBJ) $(DEMO_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/shell: $(KERNEL_OBJ) $(USER_OBJ) $(SHELL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# ------------------------------------------------------------------ #
#  Compilation (one rule covers all subdirectories)                   #
# ------------------------------------------------------------------ #

$(BUILD)/kernel/%.o: kernel/%.c | $(BUILD)/kernel
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/user/%.o: user/%.c | $(BUILD)/user
	$(CC) $(CFLAGS) -c -o $@ $<

# ------------------------------------------------------------------ #
#  Directory creation                                                  #
# ------------------------------------------------------------------ #

$(BUILD)/kernel:
	mkdir -p $@

$(BUILD)/user:
	mkdir -p $@

# ------------------------------------------------------------------ #
#  Clean                                                               #
# ------------------------------------------------------------------ #

clean:
	rm -rf $(BUILD)
