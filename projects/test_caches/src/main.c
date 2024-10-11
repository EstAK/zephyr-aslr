#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>
#include <zephyr/cache.h>
#include <stdio.h>

#define MAX_RUNS 50
#define RUNS 10
#define OFFSET 10
#define PRIORITY 5
#define USER_STACKSIZE 4096
#define HEAP_SIZE CONFIG_MMU_PAGE_SIZE*200

/* the start address of the MPU region needs to align with its size */
uint64_t __aligned(CONFIG_MMU_PAGE_SIZE) user_heap_mem[HEAP_SIZE];

K_MEM_PARTITION_DEFINE(part, user_heap_mem, sizeof(user_heap_mem),
                       K_MEM_PARTITION_P_RW_U_RW);

struct k_mem_partition *user_partitions[] = {
    &part,
};

struct k_mem_domain user_domain;
struct sys_heap user_heap;

static void make_cyclic_permutation(uint64_t n, uint64_t *array)
{
    double r;
    uint64_t theta;
    uint64_t temp;
    uint64_t i = 0;

    for (; i < n; i++) {
        array[i] = i; 
    }

    i = n-1;

    while (i != 0) {
        r = (double)rand()/(double)(RAND_MAX/1);;
        theta = (uint64_t)floor(i * r);
        temp = array[i];
        array[i] = array[theta];
        array[theta] = temp;
        i--;
    }
}

static void seq_read(volatile uint64_t *array, uint64_t size)
{
	volatile uint64_t acc = 0;
    uint64_t idx = 0;
    // i is intended to be unused
	for (uint64_t i = 0; i < size; i++) {
		acc += array[idx];
        idx = array[idx];
	}
}

static void benchmark(void *p1, void *p2, void *p3)
{
	sys_heap_init(&user_heap, &user_heap_mem[0], HEAP_SIZE);

	uint64_t start_time, end_time;
	uint64_t total_cycles = 0;
	uint64_t total_ns = 0;
	uint64_t size;
	uint64_t *array;

    uint64_t res[RUNS * MAX_RUNS];
    size_t index = 0;

	for (uint64_t i = OFFSET; i < MAX_RUNS; i++) {
		size = UINT64_C(1) << i;

		array = (uint64_t *)sys_heap_alloc(&user_heap, sizeof(uint64_t) * size);
		if (array == NULL) {
            uint64_t temp = OFFSET;
            for (int j = 0; j < (i - OFFSET) * RUNS ; j ++ ) {
                if ( j > OFFSET && j % RUNS == 0 ) {
                    temp ++;
                }
                printf("%"PRIu64",%"PRIu64"\n", (UINT64_C(1) << temp) * sizeof(uint64_t), res[j]);
            }
			printf("===END===\n");
			return;
		}

		for (unsigned int j = 0; j < RUNS; j++) {
            /*sys_cache_data_flush_and_invd_all();*/
            /*sys_cache_instr_flush_and_invd_all();*/

            make_cyclic_permutation(size, array);
			start_time = k_cycle_get_64();
			seq_read(array, size);
			end_time = k_cycle_get_64();
			total_cycles = end_time - start_time;

            res[index] = total_cycles;
            index++;

		}
		sys_heap_free(&user_heap, array);
	}
}

K_THREAD_DEFINE(user_thread, USER_STACKSIZE,
                benchmark, NULL, NULL, NULL,
                PRIORITY, 0, 0);

int main(void)
{
	printf("===START===\n");
#ifdef CONFIG_BENCHMARKING
	printf("n\n");
#else
	printf("y\n");
#endif
	k_mem_domain_init(&user_domain, 1, user_partitions);
	k_mem_domain_add_thread(&user_domain, user_thread);
	k_thread_start(user_thread);
	return 0;
}
