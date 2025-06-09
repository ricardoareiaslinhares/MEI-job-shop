#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "utils.h"
#include <string.h>
#define getClock() ((double)clock() / CLOCKS_PER_SEC)

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

void branch_bound(const InputData *input, const State *current_state, int stop_depth) {
    if (current_state->depth == stop_depth) {
        min_makespan = current_state->makespan;
        //printf("Min makespan %d\n", min_makespan);
        for (int i = 0; i < input->r; i++) {
                for (int j = 0; j < input->c; j++) {
                    best_start_time[i][j] = current_state->start_time[i][j];
                }
            }
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

int main(int argc, char *argv[]) {
            if (argc < 3) {
        printf("Usage: %s inputFile outputFile\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    FILE *filew = fopen(argv[2], "w");
    file_check(file, filew);

    InputData input = parse_file(file);
    fclose(file);
    //print_input_data(input);

    printf("Working...\n");

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

    struct timespec start_clock, end_clock;
    clock_gettime(CLOCK_MONOTONIC, &start_clock);
    for (int i = 0; i < current_unit_index; i++ ) {
        branch_bound(&input, &work_units_state[i], input.c*input.r );
    }
  clock_gettime(CLOCK_MONOTONIC, &end_clock);
    double elapsed = (end_clock.tv_sec - start_clock.tv_sec) +
                     (end_clock.tv_nsec - start_clock.tv_nsec) / 1e9;

    printf("Execution time: %.6f seconds\n", elapsed);
    printf("Best makespan: %d\n", min_makespan);
    write_to_file(filew, min_makespan, best_start_time, input.r, input.c);
    fclose(filew);
    free(work_units_state);
    return 0;

}