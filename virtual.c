#include "oslabs.h"
#include <limits.h>

int process_page_access_fifo(struct PTE page_table[], int *table_cnt, int page_number, int frame_pool[], int *frame_cnt, int current_timestamp) {
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }
    if (*frame_cnt > 0) {
        int frame = frame_pool[--(*frame_cnt)];
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        return frame;
    }
    // evict
    int min_arrival = INT_MAX;
    int victim = -1;
    for (int i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid) {
            if (page_table[i].arrival_timestamp < min_arrival) {
                min_arrival = page_table[i].arrival_timestamp;
                victim = i;
            }
        }
    }
    int freed_frame = page_table[victim].frame_number;
    page_table[victim].is_valid = 0;
    page_table[victim].frame_number = -1;
    page_table[victim].arrival_timestamp = -1;
    page_table[victim].last_access_timestamp = -1;
    page_table[victim].reference_count = -1;
    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = freed_frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;
    return freed_frame;
}

int count_page_faults_fifo(struct PTE page_table[], int table_cnt, int reference_string[], int reference_cnt, int frame_pool[], int frame_cnt) {
    int faults = 0;
    int current_timestamp = 0;
    int local_frame_cnt = frame_cnt;
    for (int i = 0; i < reference_cnt; i++) {
        current_timestamp++;
        int page = reference_string[i];
        if (page_table[page].is_valid) {
            page_table[page].last_access_timestamp = current_timestamp;
            page_table[page].reference_count++;
        } else {
            faults++;
            if (local_frame_cnt > 0) {
                int frame = frame_pool[--local_frame_cnt];
                page_table[page].is_valid = 1;
                page_table[page].frame_number = frame;
                page_table[page].arrival_timestamp = current_timestamp;
                page_table[page].last_access_timestamp = current_timestamp;
                page_table[page].reference_count = 1;
            } else {
                int min_arrival = INT_MAX;
                int victim = -1;
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid) {
                        if (page_table[j].arrival_timestamp < min_arrival) {
                            min_arrival = page_table[j].arrival_timestamp;
                            victim = j;
                        }
                    }
                }
                int freed = page_table[victim].frame_number;
                page_table[victim].is_valid = 0;
                page_table[victim].frame_number = -1;
                page_table[victim].arrival_timestamp = -1;
                page_table[victim].last_access_timestamp = -1;
                page_table[victim].reference_count = -1;
                page_table[page].is_valid = 1;
                page_table[page].frame_number = freed;
                page_table[page].arrival_timestamp = current_timestamp;
                page_table[page].last_access_timestamp = current_timestamp;
                page_table[page].reference_count = 1;
            }
        }
    }
    return faults;
}

int process_page_access_lru(struct PTE page_table[], int *table_cnt, int page_number, int frame_pool[], int *frame_cnt, int current_timestamp) {
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }
    if (*frame_cnt > 0) {
        int frame = frame_pool[--(*frame_cnt)];
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        return frame;
    }
    // evict
    int min_last = INT_MAX;
    int victim = -1;
    for (int i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid) {
            if (page_table[i].last_access_timestamp < min_last) {
                min_last = page_table[i].last_access_timestamp;
                victim = i;
            }
        }
    }
    int freed_frame = page_table[victim].frame_number;
    page_table[victim].is_valid = 0;
    page_table[victim].frame_number = -1;
    page_table[victim].arrival_timestamp = -1;
    page_table[victim].last_access_timestamp = -1;
    page_table[victim].reference_count = -1;
    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = freed_frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;
    return freed_frame;
}

int count_page_faults_lru(struct PTE page_table[], int table_cnt, int reference_string[], int reference_cnt, int frame_pool[], int frame_cnt) {
    int faults = 0;
    int current_timestamp = 0;
    int local_frame_cnt = frame_cnt;
    for (int i = 0; i < reference_cnt; i++) {
        current_timestamp++;
        int page = reference_string[i];
        if (page_table[page].is_valid) {
            page_table[page].last_access_timestamp = current_timestamp;
            page_table[page].reference_count++;
        } else {
            faults++;
            if (local_frame_cnt > 0) {
                int frame = frame_pool[--local_frame_cnt];
                page_table[page].is_valid = 1;
                page_table[page].frame_number = frame;
                page_table[page].arrival_timestamp = current_timestamp;
                page_table[page].last_access_timestamp = current_timestamp;
                page_table[page].reference_count = 1;
            } else {
                int min_last = INT_MAX;
                int victim = -1;
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid) {
                        if (page_table[j].last_access_timestamp < min_last) {
                            min_last = page_table[j].last_access_timestamp;
                            victim = j;
                        }
                    }
                }
                int freed = page_table[victim].frame_number;
                page_table[victim].is_valid = 0;
                page_table[victim].frame_number = -1;
                page_table[victim].arrival_timestamp = 0;
                page_table[victim].last_access_timestamp = 0;
                page_table[victim].reference_count = 0;
                page_table[page].is_valid = 1;
                page_table[page].frame_number = freed;
                page_table[page].arrival_timestamp = current_timestamp;
                page_table[page].last_access_timestamp = current_timestamp;
                page_table[page].reference_count = 1;
            }
        }
    }
    return faults;
}

int process_page_access_lfu(struct PTE page_table[], int *table_cnt, int page_number, int frame_pool[], int *frame_cnt, int current_timestamp) {
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }
    if (*frame_cnt > 0) {
        int frame = frame_pool[--(*frame_cnt)];
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        return frame;
    }
    // evict
    int min_ref = INT_MAX;
    int min_arrival = INT_MAX;
    int victim = -1;
    for (int i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid) {
            int ref = page_table[i].reference_count;
            int arr = page_table[i].arrival_timestamp;
            if (ref < min_ref || (ref == min_ref && arr < min_arrival)) {
                min_ref = ref;
                min_arrival = arr;
                victim = i;
            }
        }
    }
    int freed_frame = page_table[victim].frame_number;
    page_table[victim].is_valid = 0;
    page_table[victim].frame_number = -1;
    page_table[victim].arrival_timestamp = 0;
    page_table[victim].last_access_timestamp = 0;
    page_table[victim].reference_count = 0;
    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = freed_frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;
    return freed_frame;
}

int count_page_faults_lfu(struct PTE page_table[], int table_cnt, int reference_string[], int reference_cnt, int frame_pool[], int frame_cnt) {
    int faults = 0;
    int current_timestamp = 0;
    int local_frame_cnt = frame_cnt;
    for (int i = 0; i < reference_cnt; i++) {
        current_timestamp++;
        int page = reference_string[i];
        if (page_table[page].is_valid) {
            page_table[page].last_access_timestamp = current_timestamp;
            page_table[page].reference_count++;
        } else {
            faults++;
            if (local_frame_cnt > 0) {
                int frame = frame_pool[--local_frame_cnt];
                page_table[page].is_valid = 1;
                page_table[page].frame_number = frame;
                page_table[page].arrival_timestamp = current_timestamp;
                page_table[page].last_access_timestamp = current_timestamp;
                page_table[page].reference_count = 1;
            } else {
                int min_ref = INT_MAX;
                int min_arrival = INT_MAX;
                int victim = -1;
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid) {
                        int ref = page_table[j].reference_count;
                        int arr = page_table[j].arrival_timestamp;
                        if (ref < min_ref || (ref == min_ref && arr < min_arrival)) {
                            min_ref = ref;
                            min_arrival = arr;
                            victim = j;
                        }
                    }
                }
                int freed = page_table[victim].frame_number;
                page_table[victim].is_valid = 0;
                page_table[victim].frame_number = -1;
                page_table[victim].arrival_timestamp = 0;
                page_table[victim].last_access_timestamp = 0;
                page_table[victim].reference_count = 0;
                page_table[page].is_valid = 1;
                page_table[page].frame_number = freed;
                page_table[page].arrival_timestamp = current_timestamp;
                page_table[page].last_access_timestamp = current_timestamp;
                page_table[page].reference_count = 1;
            }
        }
    }
    return faults;
}
