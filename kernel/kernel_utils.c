#include "../include/kernel_utils.h"

void swap_process_out(process_t *process_ptr) {
    process_ptr->state = PROC_READY;
    process_ptr->run_flag = false;
    while (!process_ptr->run_flag) {
        pthread_cond_wait(&process_ptr->condition, &process_lock);
    }
}

void swap_process_in(process_t *process_ptr, int core_id) {
    process_ptr->state = PROC_RUNNING;
    process_ptr->run_flag = true;
    current_process_ptrs[core_id] = process_ptr;

    clock_gettime(CLOCK_REALTIME, &process_ptr->slice_expire_time);
    process_ptr->slice_expire_time.tv_sec++;

    pthread_cond_signal(&process_ptr->condition);
}

process_t *swap_in_ready_process(int core_id) {
    process_t *ready_process = find_process_by_state(PROC_READY);
    if (!ready_process) { return NULL; }
    swap_process_in(ready_process, core_id);
    return ready_process;
}

process_t *find_process_self() {
    pthread_t self = pthread_self();
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pthread_equal(process_table[i].thread, self)) {
            return &process_table[i];
        }
    }
    return NULL;
}

bool is_timeslice_expired(struct timespec *expire_time) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return (now.tv_sec > expire_time->tv_sec) || 
           (now.tv_sec == expire_time->tv_sec && now.tv_nsec >= expire_time->tv_nsec);
}

process_t *find_process_by_state(proc_state_t state) {
    static int last_checked_process = 0;

    int start_index = last_checked_process;
    do {
        last_checked_process = (last_checked_process + 1) % MAX_PROCESSES;
        if (process_table[last_checked_process].pid != 0 &&
            process_table[last_checked_process].state == state) {
            return &process_table[last_checked_process];
        }
    } while (last_checked_process != start_index);
    return NULL;
}

//returns core id if found, -1 if not
int find_idle_core() {
    for (int i = 0; i < NUM_CORES; i++) {
        if (current_process_ptrs[i] == NULL) {
            return i;
        }
    }
    return -1;
}

//returns core id if process is running, -1 if not
int find_core_for_process(process_t *process_ptr) {
    for (int i = 0; i < NUM_CORES; i++) {
        if (current_process_ptrs[i] == process_ptr) {
            return i;
        }
    }
    return -1;
}