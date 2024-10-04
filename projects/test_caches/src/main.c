/*
 * Copyright (c) TODO
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/sys_heap.h>

#define RUNS 5
#define PRIORITY 5
#define USER_STACKSIZE 4096
#define HEAP_SIZE CONFIG_MMU_PAGE_SIZE*2

/* the start address of the MPU region needs to align with its size */
uint8_t __aligned(CONFIG_MMU_PAGE_SIZE) user_heap_mem[HEAP_SIZE];

K_MEM_PARTITION_DEFINE(part, user_heap_mem, sizeof(user_heap_mem),
                       K_MEM_PARTITION_P_RW_U_RW);

struct k_mem_partition *user_partitions[] = {
    &part,
};

struct k_mem_domain user_domain;
struct sys_heap user_heap;

static void seq_read(uint8_t *array, uint64_t size)
{
	volatile uint64_t acc = 0;
	for (uint64_t i = 0; i < size; i++) {
		acc += i;
	}
}

static void benchmark(void *p1, void *p2, void *p3)
{
	printf("before initializing the sys heap\n");
	sys_heap_init(&user_heap, &user_heap_mem[0], HEAP_SIZE);
	printf("after initializing the sys heap\n");

	uint64_t start_time, end_time;
	uint64_t total_cycles = 0;
	uint64_t total_ns = 0;
	uint64_t size;
	uint8_t *array;

	for (uint64_t i = 10; i < 50; i++) {
		printf("---->running test with i -> %" PRIu64 "\n", i);
		size = UINT64_C(1) << i;

		printf("before the alloc with size = %" PRIu64 "\n", size);
		array = (uint8_t *)sys_heap_alloc(&user_heap, sizeof(uint8_t) * size);
		printf("good alloc\n");
		if (array == NULL) {
			printf("alloc error\n");
			return;
		}
		for (uint64_t j = 0; j < size; j++) {
			array[j] = (uint8_t)j;
		}

		for (unsigned int j = 0; j < RUNS; j++) {

			start_time = k_cycle_get_64();
			seq_read(array, size);
			end_time = k_cycle_get_64();

			total_cycles = end_time - start_time;
			total_ns = SYS_CLOCK_HW_CYCLES_TO_NS_AVG(total_cycles, 1);

			printf("%" PRIu64 ",%lld\n", size * 8, total_ns);
		}
		sys_heap_free(&user_heap, array);
	}
}

K_THREAD_DEFINE(user_thread, USER_STACKSIZE,
                benchmark, NULL, NULL, NULL,
                PRIORITY, 0, 0);

int main(void)
{
	k_mem_domain_init(&user_domain, 1, user_partitions);
	k_mem_domain_add_thread(&user_domain, user_thread);
	k_thread_start(user_thread);
	return 0;
}
