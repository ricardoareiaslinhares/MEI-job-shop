#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "utils.h"

#define getClock() ((double)clock() / CLOCKS_PER_SEC)

int min_makespan = INT_MAX;
int best_start_time[MAX][MAX];
pthread_mutex_t lock;


// The pthreads fn only takes one arg, so it was necessary to wrap this into a struct
// Used to assign jobs to individual threads
typedef struct {
    int start_job;
    int end_job;
    InputData* input;
} ThreadGroupArgs;

void branch_bound(
    int r,
    int c,
    int depth,
    int job_free[MAX],
    int machine_free[MAX],
    int job_op[MAX],
    int current_makespan,
    int machines[MAX][MAX],
    int times[MAX][MAX],
    int start_time[MAX][MAX]
)
{
    if (depth == r * c) {
        pthread_mutex_lock(&lock);
        if (current_makespan < min_makespan) {
            min_makespan = current_makespan;
            for (int i = 0; i < r; i++) {
                for (int j = 0; j < c; j++) {
                    best_start_time[i][j] = start_time[i][j];
                }
            }
        }
        pthread_mutex_unlock(&lock);
        return;
    }

    for (int job = 0; job < r; job++) {
        if (job_op[job] >= c)
            continue;

        int operation_number = job_op[job];
        int machine = machines[job][operation_number];
        int duration = times[job][operation_number];
        int start = job_free[job] > machine_free[machine] ? job_free[job] : machine_free[machine];
        int end = start + duration;

        if (end >= min_makespan) continue;

        int new_job_free[MAX];
        int new_machine_free[MAX];
        int new_job_op[MAX];
        int new_start_time[MAX][MAX];

        memcpy(new_job_free, job_free, sizeof(int) * MAX);
        memcpy(new_machine_free, machine_free, sizeof(int) * MAX);
        memcpy(new_job_op, job_op, sizeof(int) * MAX);
        for (int i = 0; i < MAX; i++) {
            for (int j = 0; j < MAX; j++) {
                new_start_time[i][j] = start_time[i][j];
            }
        }

        new_job_free[job] = end;
        new_machine_free[machine] = end;
        new_job_op[job]++;
        new_start_time[job][operation_number] = start;

        int new_makespan = end > current_makespan ? end : current_makespan;

        branch_bound(
            r,
            c,
            depth + 1,
            new_job_free,
            new_machine_free,
            new_job_op,
            new_makespan,
            machines,
            times,
            new_start_time);
    }
}

void* pthread_fn(void* arg) {
    // Pthreads type casting
    ThreadGroupArgs* group = (ThreadGroupArgs*)arg;

    for (int job = (*group).start_job; job < (*group).end_job; job++) {
        InputData* input = (*group).input;

        int r = (*input).r;
        int c = (*input).c;
        int depth = 0;
        int job_free[MAX] = {0};
        int machine_free[MAX] = {0};
        int job_op[MAX] = {0};
        int start_time[MAX][MAX] = {{0}};
        int machines[MAX][MAX];
        int times[MAX][MAX];

        memcpy(machines, (*input).machines, sizeof(int) * MAX * MAX);
        memcpy(times, (*input).times, sizeof(int) * MAX * MAX);

        branch_bound(
            r, c, depth,
            job_free, machine_free, job_op,
            0, machines, times, start_time);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s inputFile outputFile num_threads\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    FILE *filew = fopen(argv[2], "w");
    file_check(file, filew);

    InputData input = parse_file(file);
    fclose(file);
    print_input_data(input);
    printf("Working...\n");

    int num_threads = atoi(argv[3]);
    // Uncomment bellow to cap threads to job size
    //if (num_threads > r) num_threads = r;

    pthread_t* all_threads;
    all_threads = malloc(num_threads * sizeof(pthread_t));
    pthread_mutex_init(&lock, NULL);

    // Create an array of ThreadGroupArgs to be passed into each thread
    ThreadGroupArgs group_args[num_threads];
    // Divide jobs per threads, making sure all jobs get assigned (some t may get less if uneven total jobs)
    int jobs_per_thread = (input.r + num_threads - 1) / num_threads;

    double start = getClock();
    // Job assignment, thread creation and computation
    for (int t = 0; t < num_threads; t++) {
        // assign to the same address of variable input, safe because input is never mutated
        group_args[t].input = &input;

        // assign jobs to this (t) thread
        group_args[t].start_job = t * jobs_per_thread;
        group_args[t].end_job = (t + 1) * jobs_per_thread;

        // ensure last thread doesnt get assigned non existing jobs
        if (group_args[t].end_job > input.r) group_args[t].end_job = input.r;

        pthread_create(&all_threads[t], NULL, pthread_fn, &group_args[t]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(all_threads[i], NULL);
    }

    double end = getClock();

    printf("Execution time: %.6f\n", end - start);
    printf("Optimal makespan: %d\n", min_makespan);
    write_to_file(filew, min_makespan, best_start_time, input.r, input.c);

    fclose(filew);
    pthread_mutex_destroy(&lock);
    free(all_threads);
    return 0;
}