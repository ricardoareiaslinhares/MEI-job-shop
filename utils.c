#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

InputData parse_file(FILE *file) {
    InputData data;
    char line[256];

    // Skip initial commented lines (#)
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '#') {
            sscanf(line, "%d %d", &data.r, &data.c); // set matrix size
            break;
        }
    }

    for (int i = 0; i < data.r; i++) {
        for (int j = 0; j < data.c; j++) {
            if (fscanf(file, "%d %d", &data.machines[i][j], &data.times[i][j]) != 2) {
                printf("Invalid input at job %d, machine %d\n", i, j);
            }
        }
    }

    return data;
}


void print_input_data(InputData data) {
    printf("We have %d jobs and %d machines:\n", data.r, data.c);

    for (int i = 0; i < data.r; i++) {
        printf("Job %d:\n", i);
        for (int j = 0; j < data.c; j++) {
            printf("  Operation %d → Machine %d, Time %d\n",
                   j, data.machines[i][j], data.times[i][j]);
        }
    }
}


void write_to_file(FILE *file, int makespan, int start_time[MAX][MAX], int num_jobs, int num_machines)
{
    fprintf(file, "%d\n", makespan);

    for (int i = 0; i < num_jobs; i++) {
        for (int j = 0; j < num_machines; j++) {
            fprintf(file, "%d ", start_time[i][j]);
        }
        fprintf(file, "\n");
    }
}

// Verifies if each file was passed
int file_check(FILE *file, FILE *filew) {
    if (!file)
    {
        printf("Error opening the input file\n");
        return 1;
    }
    if (!filew)
    {
        printf("Error opening the output file\n");
        return 1;
    }
    return 0;
}