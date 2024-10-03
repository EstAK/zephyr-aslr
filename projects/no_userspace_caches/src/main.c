/*
 * Copyright (c) 2020 BayLibre, SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/timing/timing.h>

#define RUNS 5

#define USER_STACKSIZE  4096
#define HEAP_SIZE       4108864

K_HEAP_DEFINE_NOCACHE(heap, HEAP_SIZE)

static void seq_read(uint8_t* array, uint64_t size)
{
    volatile uint64_t acc = 0;
    for (uint64_t i = 0; i < size; i++) {
        acc += i;
    }
}

static void benchmark()
{
    timing_t start_time, end_time;
    uint64_t total_cycles = 0;
    uint64_t total_ns = 0;
    uint64_t size;
    uint8_t* array;

    printf("size,time\n");
    for (uint64_t i = 10; i < 50; i++) {
        size = UINT64_C(1) << i;

        array = (uint8_t*)k_heap_alloc(&heap, sizeof(uint8_t) * size, K_NO_WAIT);
        if (array == NULL) {
            printf("alloc error\n");
            return;
        }

        for (uint64_t j = 0; j < size; j++) {
            array[j] = (uint8_t)j;
        }

        for (unsigned int j = 0; j < RUNS; j++) {
            start_time = timing_counter_get();
            seq_read(array, size);

            end_time = timing_counter_get();

            total_cycles = timing_cycles_get(&start_time, &end_time);
            total_ns = timing_cycles_to_ns(total_cycles);

            printf("%" PRIu64 ",%lld\n", size * 8, total_ns);
        }
        k_heap_free(&heap, array);
    }
}

int main(void)
{
    printf("hello world\n");
    timing_init();
    timing_start();

    benchmark();

    timing_stop();

    return 0;
}
