#include "../include/kernel_utils.h"

void swap_process_out(process_t *process_ptr) {
    process_ptr->state = PROC_READY;
    process_ptr->run_flag = false;
    while (!process_ptr->run_flag) {
        pthread_cond_wait(&process_ptr->condition, &process_lock);
    }
}

void swap_process_in(process_t *process_ptr) {
    process_ptr->state = PROC_RUNNING;
    process_ptr->run_flag = true;
    current_process_ptr = process_ptr;

    clock_gettime(CLOCK_REALTIME, &process_ptr->slice_expire_time);
    process_ptr->slice_expire_time.tv_sec++;

    pthread_cond_signal(&process_ptr->condition);
}

process_t *swap_in_ready_process(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != 0 && process_table[i].state == PROC_READY) {
            swap_process_in(&process_table[i]);
            return &process_table[i];
        }
    }
    return NULL;
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