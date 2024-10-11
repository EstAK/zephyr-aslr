/*
 * Copyright (c) TODO
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/sys_heap.h>

#define RUNS 100
#define PRIORITY 5

#define USER_STACKSIZE 16384
#define SIZE 30

unsigned int dfs(uint64_t graph[SIZE][SIZE], uint64_t source, uint64_t destination)
{
    unsigned int res;
    if (graph[source][destination] == 1) {
        return 1;
    }
    for (uint64_t i = 0; i < SIZE; i++) {
        if (graph[source][i] == 1) {
            if ((res = dfs(graph, i, destination)) == 1) {
                return res;
            }
        }
    }
    return 0;
}

void populate_graph(uint64_t graph[SIZE][SIZE])
{
    unsigned int visited[SIZE] = { 0 };
    uint64_t temp, pair;

    temp = sys_rand64_get() % SIZE;
    pair = sys_rand64_get() % SIZE;

    visited[pair] = 1;
    visited[temp] = 1;

    graph[temp][pair] = 1;
    graph[pair][temp] = 1;

    for (uint64_t i = 0; i < SIZE; i++) {
        if (visited[i] == 1) {
            continue;
        }

        pair = sys_rand64_get() % SIZE; 
        while (pair == i || visited[pair] != 1) {
            pair++;
            pair %= SIZE;
        }

        // undirected graph
        graph[i][pair] = 1;
        graph[pair][i] = 1;

        pair = (pair + 1) % SIZE;
    }
}

static void benchmark(void* user_heap_mem, void* user_heap, void* p3)
{
    uint64_t start_time, end_time;
    uint64_t total_cycles = 0;
    unsigned int g[SIZE][SIZE];

    for (uint64_t i = 0; i < RUNS; i ++) {
        dfs(g, sys_rand64_get() % SIZE, sys_rand64_get() % SIZE);

        printf("%"PRIu64"\n", k_cycle_get_64());
        k_thread_priority_set(_current, k_thread_priority_get(_current));
        printf("%"PRIu64"\n", k_cycle_get_64());
    }
}

static void benchmark2(void* user_heap_mem, void* user_heap, void* p3)
{
    uint64_t start_time, end_time;
    uint64_t total_cycles = 0;
    unsigned int g[SIZE][SIZE];

    for (uint64_t i = 0; i < RUNS; i ++) {
        dfs(g, sys_rand64_get() % SIZE, sys_rand64_get() % SIZE);

        printf("%"PRIu64"\n", k_cycle_get_64());
        k_thread_priority_set(_current, k_thread_priority_get(_current));
        printf("%"PRIu64"\n", k_cycle_get_64());
    }
}

K_THREAD_DEFINE(user1_thread, USER_STACKSIZE,
                benchmark, NULL, NULL, NULL,

                PRIORITY, 0, 0);

K_THREAD_DEFINE(user2_thread, USER_STACKSIZE,
                benchmark2, NULL, NULL, NULL,
                PRIORITY, 0, 0);

int main(void)
{

    printf("===START===\n");
#ifdef CONFIG_EXPERIMENTAL_ASLR
    printf("y\n");
#else
    printf("n\n");
#endif

/*#ifdef CONFIG_BENCHMARKING*/
/*    printf("n\n");*/
/*#else*/
/*    printf("y\n");*/
/*#endif*/
    k_thread_start(user1_thread);
    k_thread_start(user2_thread);

    k_thread_join(user1_thread,K_FOREVER);
    k_thread_join(user2_thread, K_FOREVER);
    return 0;
}
