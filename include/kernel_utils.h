#include "kernel.h"

//make this process ready to run, but not running currently
//NOTE: process_lock should be engaged before calling this
//note: it is not ensured that swap_process_out will precede swap_process_in when changing processes
void swap_process_out(process_t *process_ptr);

//pre: process_ptr is PROC_READY, process_lock is engaged
void swap_process_in(process_t *process_ptr);

//if possible, swap in a process that 1. exists (nonzero PID) and 2. is ready to run (PROC_READY)
//pre: process_lock is engaged
process_t *swap_in_ready_process(void);

//determine thread when current_process_ptr isn't initalized
process_t *find_process_self();

bool is_timeslice_expired(struct timespec *expire_time);