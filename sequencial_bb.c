#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "utils.h"
#include <string.h>
#define getClock() ((double)clock() / CLOCKS_PER_SEC)

int min_makespan = INT_MAX;
int best_start_time[MAX][MAX];

void branch_bound(
    int r,                 // number of rows (Jobs)
    int c,                 // number of columns (Machines)
    int depth,             // matrix size r*x
    int job_free[MAX],     // when can job r start
    int machine_free[MAX], // when is machine c free
    int job_op[MAX],       // how many ops for job r
    int current_makespan,
    int machines[MAX][MAX],
    int times[MAX][MAX],
    int start_time[MAX][MAX])
{
    // Backtracks / exit condition
    if (depth == r * c)
    {
        if (current_makespan < min_makespan)
        {
            min_makespan = current_makespan;
                    printf("Min makespan %d\n", min_makespan);
            for (int i = 0; i < r; i++)
            {
                for (int j = 0; j < c; j++)
                {
                    best_start_time[i][j] = start_time[i][j];
                }
            }
        }
        return;
    }

    for (int job = 0; job < r; job++)
    {
        // Job has no more operations if it passed trough all machines
        if (job_op[job] >= c)
            continue; // so skips this job

        int operation_number = job_op[job];
        int machine = machines[job][operation_number];
        int duration = times[job][operation_number];

        // printf("Operation #%d of Job %d is at machine %d and it will take %d\n", operation_number, job, machine, duration);

        int start = job_free[job] > machine_free[machine] ? job_free[job] : machine_free[machine];
        int end = start + duration;

        // If end is already bigger than the min_makespan found so far, it is not worth going deeper, so skips
        if (end >= min_makespan)
        {
            // printf("Eliminated route of job %d at Op %d that ends at %d, while the best time is %d\n", job, operation_number, end, min_makespan );
            continue;
        }

        // Initiate variables for next recursion
        int new_job_free[MAX];
        int new_machine_free[MAX];
        int new_job_op[MAX];
        int new_start_time[MAX][MAX];

        memcpy(new_job_free, job_free, sizeof(int) * MAX);
        memcpy(new_machine_free, machine_free, sizeof(int) * MAX);
        memcpy(new_job_op, job_op, sizeof(int) * MAX);
        for (int i = 0; i < MAX; i++)
        {
            for (int j = 0; j < MAX; j++)
            {
                new_start_time[i][j] = start_time[i][j];
            }
        }

        // Update variables for the next recursion
        new_job_free[job] = end;
        new_machine_free[machine] = end;
        new_job_op[job]++;
        new_start_time[job][operation_number] = start;

        // Because the makespan is the total time of all jobs, we track the longer we have seen so far
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

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s pass inputFile and an outputFile as args\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    FILE *filew = fopen(argv[2], "w");
    file_check(file, filew);

    InputData input = parse_file(file);
    fclose(file);
    print_input_data(input);
    printf("Working...\n");

    // Initiate variables for the branch_and_bound
    int r = input.r, c = input.c;
    // All jobs start at the first operation (/machine) so initialize with 0
    int job_free[MAX] = {0};
    int machine_free[MAX] = {0};
    int job_op[MAX] = {0};
    int start_time[MAX][MAX];
    for (int i = 0; i < MAX; i++)
    {
        for (int j = 0; j < MAX; j++)
        {
            start_time[i][j] = 0;
        }
    }

    struct timespec start_clock, end_clock;
    clock_gettime(CLOCK_MONOTONIC, &start_clock);
    branch_bound(
        r,
        c,
        0, // depth
        job_free,
        machine_free,
        job_op,
        0, // current makespan
        input.machines,
        input.times,
        start_time

    );
    clock_gettime(CLOCK_MONOTONIC, &end_clock);
    double elapsed = (end_clock.tv_sec - start_clock.tv_sec) +
                     (end_clock.tv_nsec - start_clock.tv_nsec) / 1e9;

    printf("Execution time: %.6f seconds\n", elapsed);
    printf("Best makespan: %d\n", min_makespan);
    write_to_file(filew, min_makespan, best_start_time, input.r, input.c);
    fclose(filew);
    return 0;
}
