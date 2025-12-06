#include "oslabs.h"

/* =====================================================================
   ======================  FIFO 页面访问处理  ======================
   ===================================================================== */
int process_page_access_fifo(struct PTE page_table[TABLEMAX],
                             int *table_cnt,
                             int page_number,
                             int frame_pool[POOLMAX],
                             int *frame_cnt,
                             int current_timestamp) {

    // 情况1：页面已在内存中（有效）
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }

    // 情况2：页面不在内存中，但有空闲帧
    if (*frame_cnt > 0) {
        // 从帧池中取出第一个空闲帧（注意：不是最后一个）
        int free_frame = frame_pool[0];
        
        // 将帧池中的元素前移
        for (int i = 0; i < *frame_cnt - 1; i++) {
            frame_pool[i] = frame_pool[i + 1];
        }
        (*frame_cnt)--;

        // 分配该帧给请求的页面
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = free_frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_table[page_number].reference_count = 1;

        return free_frame;
    }

    // 情况3：页面不在内存中，且无空闲帧 -> 需要置换
    int victim_index = -1;
    int earliest_arrival = 1000000000; // 使用一个大数初始化

    // 遍历页表，寻找有效页面中到达时间最早的（FIFO策略）
    for (int i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid) {
            if (page_table[i].arrival_timestamp < earliest_arrival) {
                earliest_arrival = page_table[i].arrival_timestamp;
                victim_index = i;
            }
            // 如果到达时间相同，选择索引更小的（确保确定性）
            else if (page_table[i].arrival_timestamp == earliest_arrival && i < victim_index) {
                victim_index = i;
            }
        }
    }

    // 正常情况下victim_index不应为-1
    int victim_frame = page_table[victim_index].frame_number;

    // 将置换出的页面标记为无效，并重置其字段为-1
    page_table[victim_index].is_valid = 0;
    page_table[victim_index].frame_number = -1;
    page_table[victim_index].arrival_timestamp = -1;
    page_table[victim_index].last_access_timestamp = -1;
    page_table[victim_index].reference_count = -1;

    // 将释放出的帧分配给新请求的页面
    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = victim_frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;

    return victim_frame;
}

/* =====================================================================
   =====================  FIFO 缺页计数模拟  =======================
   ===================================================================== */
int count_page_faults_fifo(struct PTE page_table[TABLEMAX],
                           int table_cnt,
                           int reference_string[REFERENCEMAX],
                           int reference_cnt,
                           int frame_pool[POOLMAX],
                           int frame_cnt) {

    int faults = 0;
    int current_timestamp = 1;
    
    // 创建本地副本，避免修改原始帧池
    int local_frame_pool[POOLMAX];
    int local_frame_cnt = frame_cnt;
    for (int i = 0; i < frame_cnt; i++) {
        local_frame_pool[i] = frame_pool[i];
    }

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        
        // 检查页面是否已在内存
        if (!page_table[page_number].is_valid) {
            faults++; // 发生缺页
            
            if (local_frame_cnt > 0) {
                // 使用空闲帧
                int free_frame = local_frame_pool[0];
                // 前移帧池元素
                for (int j = 0; j < local_frame_cnt - 1; j++) {
                    local_frame_pool[j] = local_frame_pool[j + 1];
                }
                local_frame_cnt--;
                
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = free_frame;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            } else {
                // 需要置换：选择到达时间最早的页面
                int victim_index = -1;
                int earliest_arrival = 1000000000;
                
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid) {
                        if (page_table[j].arrival_timestamp < earliest_arrival) {
                            earliest_arrival = page_table[j].arrival_timestamp;
                            victim_index = j;
                        }
                        else if (page_table[j].arrival_timestamp == earliest_arrival && j < victim_index) {
                            victim_index = j;
                        }
                    }
                }
                
                int victim_frame = page_table[victim_index].frame_number;
                
                // 重置换出页面
                page_table[victim_index].is_valid = 0;
                page_table[victim_index].frame_number = -1;
                page_table[victim_index].arrival_timestamp = -1;
                page_table[victim_index].last_access_timestamp = -1;
                page_table[victim_index].reference_count = -1;
                
                // 分配帧给新页面
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = victim_frame;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            }
        } else {
            // 页面命中，更新访问信息
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count++;
        }
        
        current_timestamp++;
    }

    return faults;
}

/* =====================================================================
   ======================  LRU 页面访问处理  ======================
   ===================================================================== */
int process_page_access_lru(struct PTE page_table[TABLEMAX],
                            int *table_cnt,
                            int page_number,
                            int frame_pool[POOLMAX],
                            int *frame_cnt,
                            int current_timestamp) {

    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }

    if (*frame_cnt > 0) {
        int free_frame = frame_pool[0];
        for (int i = 0; i < *frame_cnt - 1; i++) {
            frame_pool[i] = frame_pool[i + 1];
        }
        (*frame_cnt)--;

        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = free_frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;

        return free_frame;
    }

    // LRU置换：选择最近最久未访问的页面（last_access_timestamp最小）
    int victim_index = -1;
    int oldest_access = 1000000000;

    for (int i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid) {
            if (page_table[i].last_access_timestamp < oldest_access) {
                oldest_access = page_table[i].last_access_timestamp;
                victim_index = i;
            }
            else if (page_table[i].last_access_timestamp == oldest_access && i < victim_index) {
                victim_index = i;
            }
        }
    }

    int victim_frame = page_table[victim_index].frame_number;

    // LRU换出页面字段置为-1
    page_table[victim_index].is_valid = 0;
    page_table[victim_index].frame_number = -1;
    page_table[victim_index].arrival_timestamp = -1;
    page_table[victim_index].last_access_timestamp = -1;
    page_table[victim_index].reference_count = -1;

    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = victim_frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;

    return victim_frame;
}

/* =====================================================================
   =====================  LRU 缺页计数模拟  =======================
   ===================================================================== */
int count_page_faults_lru(struct PTE page_table[TABLEMAX],
                          int table_cnt,
                          int reference_string[REFERENCEMAX],
                          int reference_cnt,
                          int frame_pool[POOLMAX],
                          int frame_cnt) {

    int faults = 0;
    int current_timestamp = 1;
    
    // 创建本地帧池副本
    int local_frame_pool[POOLMAX];
    int local_frame_cnt = frame_cnt;
    for (int i = 0; i < frame_cnt; i++) {
        local_frame_pool[i] = frame_pool[i];
    }

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        
        if (!page_table[page_number].is_valid) {
            faults++;
            
            if (local_frame_cnt > 0) {
                int free_frame = local_frame_pool[0];
                for (int j = 0; j < local_frame_cnt - 1; j++) {
                    local_frame_pool[j] = local_frame_pool[j + 1];
                }
                local_frame_cnt--;
                
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = free_frame;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            } else {
                // LRU置换：选择最近最久未访问的页面
                int victim_index = -1;
                int oldest_access = 1000000000;
                
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid) {
                        if (page_table[j].last_access_timestamp < oldest_access) {
                            oldest_access = page_table[j].last_access_timestamp;
                            victim_index = j;
                        }
                        else if (page_table[j].last_access_timestamp == oldest_access && j < victim_index) {
                            victim_index = j;
                        }
                    }
                }
                
                int victim_frame = page_table[victim_index].frame_number;
                
                // 注意：在LRU计数函数中，换出页面字段设置为0（不是-1）
                page_table[victim_index].is_valid = 0;
                page_table[victim_index].frame_number = -1;
                page_table[victim_index].arrival_timestamp = 0;
                page_table[victim_index].last_access_timestamp = 0;
                page_table[victim_index].reference_count = 0;
                
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = victim_frame;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            }
        } else {
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count++;
        }
        
        current_timestamp++;
    }

    return faults;
}

/* =====================================================================
   ======================  LFU 页面访问处理  ======================
   ===================================================================== */
int process_page_access_lfu(struct PTE page_table[TABLEMAX],
                            int *table_cnt,
                            int page_number,
                            int frame_pool[POOLMAX],
                            int *frame_cnt,
                            int current_timestamp) {

    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }

    if (*frame_cnt > 0) {
        int free_frame = frame_pool[0];
        for (int i = 0; i < *frame_cnt - 1; i++) {
            frame_pool[i] = frame_pool[i + 1];
        }
        (*frame_cnt)--;

        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = free_frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;

        return free_frame;
    }

    // LFU置换：选择引用计数最小的，相同则选到达时间最早的
    int victim_index = -1;
    int min_reference = 1000000000;
    int earliest_arrival = 1000000000;

    for (int i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid) {
            if (page_table[i].reference_count < min_reference) {
                min_reference = page_table[i].reference_count;
                earliest_arrival = page_table[i].arrival_timestamp;
                victim_index = i;
            }
            else if (page_table[i].reference_count == min_reference) {
                if (page_table[i].arrival_timestamp < earliest_arrival) {
                    earliest_arrival = page_table[i].arrival_timestamp;
                    victim_index = i;
                }
                else if (page_table[i].arrival_timestamp == earliest_arrival && i < victim_index) {
                    victim_index = i;
                }
            }
        }
    }

    int victim_frame = page_table[victim_index].frame_number;

    // LFU换出页面字段设置为0（根据样例输出）
    page_table[victim_index].is_valid = 0;
    page_table[victim_index].frame_number = -1;
    page_table[victim_index].arrival_timestamp = 0;
    page_table[victim_index].last_access_timestamp = 0;
    page_table[victim_index].reference_count = 0;

    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = victim_frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;

    return victim_frame;
}

/* =====================================================================
   =====================  LFU 缺页计数模拟  =======================
   ===================================================================== */
int count_page_faults_lfu(struct PTE page_table[TABLEMAX],
                          int table_cnt,
                          int reference_string[REFERENCEMAX],
                          int reference_cnt,
                          int frame_pool[POOLMAX],
                          int frame_cnt) {

    int faults = 0;
    int current_timestamp = 1;
    
    // 创建本地帧池副本
    int local_frame_pool[POOLMAX];
    int local_frame_cnt = frame_cnt;
    for (int i = 0; i < frame_cnt; i++) {
        local_frame_pool[i] = frame_pool[i];
    }

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        
        if (!page_table[page_number].is_valid) {
            faults++;
            
            if (local_frame_cnt > 0) {
                int free_frame = local_frame_pool[0];
                for (int j = 0; j < local_frame_cnt - 1; j++) {
                    local_frame_pool[j] = local_frame_pool[j + 1];
                }
                local_frame_cnt--;
                
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = free_frame;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            } else {
                // LFU置换：选择引用计数最小的，相同则选到达时间最早的
                int victim_index = -1;
                int min_reference = 1000000000;
                int earliest_arrival = 1000000000;
                
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid) {
                        if (page_table[j].reference_count < min_reference) {
                            min_reference = page_table[j].reference_count;
                            earliest_arrival = page_table[j].arrival_timestamp;
                            victim_index = j;
                        }
                        else if (page_table[j].reference_count == min_reference) {
                            if (page_table[j].arrival_timestamp < earliest_arrival) {
                                earliest_arrival = page_table[j].arrival_timestamp;
                                victim_index = j;
                            }
                            else if (page_table[j].arrival_timestamp == earliest_arrival && j < victim_index) {
                                victim_index = j;
                            }
                        }
                    }
                }
                
                int victim_frame = page_table[victim_index].frame_number;
                
                // LFU换出页面字段设置为0
                page_table[victim_index].is_valid = 0;
                page_table[victim_index].frame_number = -1;
                page_table[victim_index].arrival_timestamp = 0;
                page_table[victim_index].last_access_timestamp = 0;
                page_table[victim_index].reference_count = 0;
                
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = victim_frame;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            }
        } else {
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count++;
        }
        
        current_timestamp++;
    }

    return faults;
}
