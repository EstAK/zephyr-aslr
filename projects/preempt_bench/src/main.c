/*
 * Copyright (c) TODO
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/sys_heap.h>


#define SIZE 5
#define RUNS 30
#define PRIORITY 5
#define DISPLAY 0
#define USER_STACKSIZE 16384

unsigned int dfs(unsigned int graph[SIZE][SIZE],
    unsigned int source,
    unsigned int destination,
    unsigned int visited[SIZE])
{
    unsigned int res;
    if (source == destination) {
        return 1;
    }

    for (unsigned int i = 0; i < SIZE; i++) {
        if (visited[i] == 0 && graph[source][i] == 1) {
            visited[i] = 1;
            if ((res = dfs(graph, i, destination, visited)) == 1) {
                return res;
            }
        }
    }
    return 0;
}

void populate_graph(unsigned int graph[SIZE][SIZE])
{
    unsigned int visited[SIZE] = { 0 };
    unsigned int frontier[SIZE] = { 0 };
    unsigned int n_frontier = 2;

    unsigned int u = sys_rand32_get() % SIZE;
    unsigned int v = sys_rand32_get() % SIZE;

    frontier[0] = u; 
    frontier[1] = v;

    visited[u] = 1;
    visited[v] = 1;

    // undirected graph
    graph[u][v] = 1;
    graph[v][u] = 1;

    for (unsigned int i = n_frontier; i < SIZE; i++) {
        u = sys_rand32_get() % SIZE;
        while (visited[u] == 1) {
            u = (u + 1) % SIZE; 
        }
        v = frontier[sys_rand32_get() % n_frontier];

        visited[u] = 1;
        frontier[n_frontier] = u;
        n_frontier++;

        // undirected graph
        graph[u][v] = 1;
        graph[v][u] = 1;
    }
}

void display(unsigned int graph[SIZE][SIZE]) 
{
    for (unsigned int i = 0 ; i < SIZE; i ++ ) {
        for (unsigned int j = 0 ; j < SIZE; j ++ ) {
            printf("%d ", graph[i][j]);
        }
            printf("\n");
    }
}

static void benchmark(void* user_heap_mem, void* user_heap, void* p3)
{
    uint64_t start_time, end_time;
    unsigned int g[SIZE][SIZE] = {0};
    unsigned int visited[SIZE] = {0};
    uint64_t total_cycles = 0;

    populate_graph(g);
    for (unsigned int i = 0; i < RUNS; i++) {
#if DISPLAY
    display(g);
#endif
        for (unsigned j = 0; j < SIZE; j++) {
            visited[j] = 0;
        }
        dfs(g, (unsigned int)sys_rand32_get() % SIZE, (unsigned int)sys_rand32_get() % SIZE, visited);

        printf("%"PRIu64"\n", k_cycle_get_64());
        k_thread_priority_set(_current, k_thread_priority_get(_current));
        printf("%"PRIu64"\n", k_cycle_get_64());
    }
}  

K_THREAD_DEFINE(user1_thread, USER_STACKSIZE,
    benchmark, NULL, NULL, NULL,
    PRIORITY, 0, 0);

K_THREAD_DEFINE(user2_thread, USER_STACKSIZE,
    benchmark, NULL, NULL, NULL,
    PRIORITY, 0, 0);

int main(void)
{
    printf("===START===\n");
#ifdef CONFIG_EXPERIMENTAL_ASLR
    printf("y\n");
#else
    printf("n\n");
#endif
#ifdef CONFIG_BENCHMARKING
    printf("n\n");
#else
    printf("y\n");
#endif

    k_thread_start(user1_thread);
    k_thread_start(user2_thread);

    k_thread_join(user1_thread,K_FOREVER);
    k_thread_join(user2_thread, K_FOREVER);
    return 0;
}
