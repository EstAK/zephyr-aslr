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

/*#define HEAP_SIZE CONFIG_MMU_PAGE_SIZE*1938*/
#define HEAP_SIZE CONFIG_MMU_PAGE_SIZE * 100

/* the start address of the MPU region needs to align with its size */
uint8_t __aligned(CONFIG_MMU_PAGE_SIZE) user1_heap_mem[HEAP_SIZE];
uint8_t __aligned(CONFIG_MMU_PAGE_SIZE) user2_heap_mem[HEAP_SIZE];

K_MEM_PARTITION_DEFINE(part1, user1_heap_mem, sizeof(user1_heap_mem),
                       K_MEM_PARTITION_P_RW_U_RW);

K_MEM_PARTITION_DEFINE(part2, user2_heap_mem, sizeof(user2_heap_mem),
                       K_MEM_PARTITION_P_RW_U_RW);

struct k_mem_partition *user1_partitions[] = {
    &part1,
};

struct k_mem_partition *user2_partitions[] = {
    &part2,
};

struct k_mem_domain user1_domain;
struct k_mem_domain user2_domain;

struct sys_heap user1_heap;
struct sys_heap user2_heap;

static void seq_read(uint8_t *array, uint64_t size)
{
	volatile uint64_t acc = 0;
	for (uint64_t i = 0; i < size; i++) {
		acc += i;
	}
}

static void benchmark(void *user_heap_mem, void *user_heap, void *p3)
{
	sys_heap_init((struct sys_heap*)user_heap, (struct sys_heap*)user_heap_mem, HEAP_SIZE);

	uint64_t start_time, end_time;
	uint64_t total_cycles = 0;
	uint64_t total_ns = 0;
	uint64_t size;
	uint8_t *array;

	// busy work
	for (uint64_t i = 10; i < 50; i++) {
		printf("%s took over at %"PRIu64"\n",_current->name, k_cycle_get_64());
		size = UINT64_C(1) << i;

		array = (uint8_t *)sys_heap_alloc((struct sys_heap *)user_heap, sizeof(uint8_t) * size);
		if (array == NULL) {
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

			/*printf("%lld,%" PRIu64 ",%lld\n", i, size * 8, total_ns);*/
		}
		// end of busy work

		sys_heap_free((struct sys_heap*)user_heap, array);
		printf("%s yields at %"PRIu64"\n",_current->name, k_cycle_get_64());
		k_thread_priority_set(_current, k_thread_priority_get(_current));
		printf("%s is back %"PRIu64"\n",_current->name, k_cycle_get_64());
	}
}

K_THREAD_DEFINE(user1_thread, USER_STACKSIZE,
                benchmark, &user1_heap_mem, &user1_heap, NULL,

                PRIORITY, 0, 0);

K_THREAD_DEFINE(user2_thread, USER_STACKSIZE,
                benchmark, &user2_heap_mem, &user2_heap, NULL,
                PRIORITY, 0, 0);

int main(void)
{

#ifdef CONFIG_EXPERIMENTAL_ASLR
	printf("ASLR enabled\n");
#else
	printf("ASLR disabled\n");
#endif

#ifdef CONFIG_BENCHMARKING
	printf("no caches\n");
#else
	printf("caches\n");
#endif
	k_mem_domain_init(&user1_domain, 1, user1_partitions);
	k_mem_domain_add_thread(&user1_domain, user2_thread);
	k_thread_start(user1_thread);

	k_mem_domain_init(&user2_domain, 1, user2_partitions);
	k_mem_domain_add_thread(&user2_domain, user2_thread);
	k_thread_start(user2_thread);
	return 0;
}
