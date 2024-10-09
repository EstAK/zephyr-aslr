/*
 * Copyright (c) TODO
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/sys_heap.h>

#define RUNS 5
#define PRIORITY 5

#define USER_STACKSIZE 16384
#define SIZE 5

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

        while (visited[pair] != 1) {
            pair = sys_rand64_get() % SIZE;
        }

        // undirected graph
        graph[i][pair] = 1;
        graph[pair][i] = 1;

        pair = (pair + 1) % SIZE;
    }
}

static void benchmark(void* user_heap_mem, void* user_heap, void* p3)
{
    // DFS

    uint64_t start_time, end_time;
    uint64_t total_cycles = 0;
    unsigned int g[SIZE][SIZE];
    // populate_graph(&g);

    while (1) {
        start_time = k_cycle_get_64();
        end_time = k_cycle_get_64();

        total_cycles = end_time - start_time;

        printf("%s yields at %" PRIu64 "\n", _current->name, k_cycle_get_64());
        k_thread_priority_set(_current, k_thread_priority_get(_current));
        printf("%s is back %" PRIu64 "\n", _current->name, k_cycle_get_64());
    }
}

/*K_THREAD_DEFINE(user1_thread, USER_STACKSIZE,*/
/*                benchmark, NULL, NULL, NULL,*/
/**/
/*                PRIORITY, 0, 0);*/
/**/
/*K_THREAD_DEFINE(user2_thread, USER_STACKSIZE,*/
/*                benchmark, NULL, NULL, NULL,*/
/*                PRIORITY, 0, 0);*/
/**/
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
    uint64_t g[SIZE][SIZE] = { 0 };
    populate_graph(g);
    for (uint64_t i = 0; i < SIZE; i++) {
        for (uint64_t j = 0; j < SIZE; j++) {
            printf("%" PRIu64 " ", g[i][j]);
        }
        printf("\n");
    }
    printf("%d\n", dfs(g, 0, 1));
    /*k_thread_start(user1_thread);*/
    /*k_thread_start(user2_thread);*/
    return 0;
}
