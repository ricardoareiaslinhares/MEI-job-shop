#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "utils.h"

#define getClock() ((double)clock() / CLOCKS_PER_SEC)

pthread_mutex_t lock;
int min_makespan = INT_MAX;
int best_start_time[MAX][MAX];
#define STARTING_BRANCH 2 // Treads receives work from this branch level
#define MAX_WORK_UNITS 4096 

typedef struct {
    int depth;
    int job_free[MAX];
    int machine_free[MAX];
    int job_op[MAX];
    int makespan;
    int start_time[MAX][MAX];
} State;
// Passing by reference to avoid waste memory on copy
void branch_bound(const InputData *input, const State *current_state, int stop_depth) {
    if (current_state->depth == stop_depth) {
        pthread_mutex_lock(&lock);
        min_makespan = current_state->makespan;
        for (int i = 0; i < input->r; i++) {
                for (int j = 0; j < input->c; j++) {
                    best_start_time[i][j] = current_state->start_time[i][j];
                }
            }
        pthread_mutex_unlock(&lock);
        return;
    }
    for (int job = 0; job < input->r; job++) {
        if (current_state->job_op[job] >= input->c) continue;

        int operation_number = current_state->job_op[job];
        int machine = input->machines[job][operation_number];
        int duration = input->times[job][operation_number];
        int start = current_state->job_free[job] > current_state->machine_free[machine] ? current_state->job_free[job] : current_state->machine_free[machine];
        int end = start + duration;

        if (end >= min_makespan) continue;

        State new_state = *current_state;

        new_state.job_free[job] = end;
        new_state.machine_free[machine] = end;
        new_state.job_op[job]++;
        new_state.depth++;
        new_state.start_time[job][operation_number] = start;
        new_state.makespan = end > current_state->makespan ? end : current_state->makespan;

        branch_bound(input, &new_state, input->r * input->c);
    }

}

// We need to run the algorith a bit (until Starting_branch) to get diferent branch to distrubete between the threads
// This Fn will recursvly search for solution up to Starting_branch, it will save all those states to be disbruted to the threads
void travel_and_generate_work(
    const InputData *input,
    const State *current_state,
    int stop_depth,
    State *units, // pointer to an array of States to explore
    int *count // to help populate those states
) {
    if (current_state->depth == stop_depth) {
        // Save current state
        units[*count] = *current_state;
        (*count)++;
        return;
    }
    for (int job = 0; job < input->r; job++) {
        if (current_state->job_op[job] >= input->c) continue;

        int operation_number = current_state->job_op[job];
        int machine = input->machines[job][operation_number];
        int duration = input->times[job][operation_number];
        int start = current_state->job_free[job] > current_state->machine_free[machine] ? current_state->job_free[job] : current_state->machine_free[machine];
        int end = start + duration;


        State new_state = *current_state;

        new_state.job_free[job] = end;
        new_state.machine_free[machine] = end;
        new_state.job_op[job]++;
        new_state.depth++;
        new_state.start_time[job][operation_number] = start;
        new_state.makespan = end > current_state->makespan ? end : current_state->makespan;

        travel_and_generate_work(input, &new_state, stop_depth, units, count);
    }


}
typedef struct {
    const InputData *input;
    State *state;
    int stop_depth;
    int start;
    int end;
} ThreadArgs;

void* thread_fn(void *arg) {
    ThreadArgs *args = (ThreadArgs*)arg;
    for (int i = args->start; i < args->end; ++i) {
        State local = args->state[i]; // make a local copy
        branch_bound(args->input, &local, args->stop_depth);
    }
    free(args);
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
    //print_input_data(input);

    printf("Working...\n");

    pthread_mutex_init(&lock, NULL);

    State initial_state;
    initial_state.depth = 0;
    initial_state.makespan = 0;
    // memset populates everything with 0
    memset(initial_state.job_free, 0, sizeof(initial_state.job_free));
    memset(initial_state.machine_free, 0, sizeof(initial_state.machine_free));
    memset(initial_state.job_op, 0, sizeof(initial_state.job_op));
    memset(initial_state.start_time, 0, sizeof(initial_state.start_time));

    State *work_units_state = malloc(MAX_WORK_UNITS * sizeof(State));
    int current_unit_index = 0;
    travel_and_generate_work(&input, &initial_state, STARTING_BRANCH, work_units_state, &current_unit_index );

    int num_threads = atoi(argv[3]);
    if (num_threads < 1) num_threads = 1;

    // This caps threads to the number of threads actual needed
    //if (num_threads > current_unit_index) num_threads = current_unit_index; 

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    struct timespec start_clock, end_clock;
    clock_gettime(CLOCK_MONOTONIC, &start_clock);
    for (int t = 0; t < num_threads; t++) {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->input = &input;
        args->stop_depth = input.r * input.c;
        args->start = t * current_unit_index / num_threads;
        args->end = (t+1) * current_unit_index / num_threads;
        args->state = work_units_state;

        pthread_create(&threads[t], NULL, thread_fn, args);
    }

    for (int t = 0; t < num_threads; ++t)
    pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end_clock);
    double elapsed = (end_clock.tv_sec - start_clock.tv_sec) +
                     (end_clock.tv_nsec - start_clock.tv_nsec) / 1e9;

    printf("Execution time: %.6f seconds\n", elapsed);
    printf("Optimal makespan: %d\n", min_makespan);
    write_to_file(filew, min_makespan, best_start_time, input.r, input.c);

    fclose(filew);
    pthread_mutex_destroy(&lock);
    free(threads);
    free(work_units_state);
    return 0;
}