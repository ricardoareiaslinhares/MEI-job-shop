/* 
JOB SHOP key points:
Cada job tem a sua própria sequencia de operações em máquinas, a sequência deve ser respeitada
 - a operação numa máquina deve terminar antes da próxima poder começar
 - cada operação tem um tempo (t) específico
 - as operações são independentes umas das outras
Uma máquina só executa um job de cada vez
*/

/* 
SEQUENCIAL
Chegar a uma solução válida e apresentar os tempos (t)
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "utils.h"
#define getClock() ((double)clock() / CLOCKS_PER_SEC)

int main(int argc, char *argv[]) {
    FILE *file = fopen(argv[1], "r");
    FILE *filew = fopen(argv[2], "w");
    InputData input = parse_file(file);
    print_input_data(input);
    printf("Working...\n");

    int start_time[MAX][MAX];

    int machine_ready[MAX] = {0}; // initiate array of MAx_MACHINES size, with zeros
    int job_ready[MAX] = {0};

    double start = getClock();
    for (int i = 0; i < input.r; i++) {
        for (int j = 0; j < input.c; j++) {
            int machine_id = input.machines[i][j];
            int op_time = input.times[i][j];

            int earliest_start = job_ready[i] > machine_ready[machine_id] ? job_ready[i] : machine_ready[machine_id];

            start_time[i][j] = earliest_start;

            job_ready[i] = earliest_start + op_time;
            machine_ready[machine_id] = earliest_start + op_time;
        }
    }

    int makespan = 0;

    // Prints starttimes for each job operation and calculates makespan
    for (int i = 0; i < input.r; i++) {
            // printf("Job %d: ", i);
        for (int j = 0; j < input.c; j++) {
           // printf("%d ", start_time[i][j]);
            int end_time = start_time[i][j] + input.times[i][j];
            if (end_time > makespan) {
                makespan = end_time;
            }
        }
        //printf("\n");
    }
    double end = getClock();
    printf("Execution time: %.6f\n", end - start);
    printf("Makespan - to complete all Jobs it takes: %d\n", makespan);
    write_to_file(filew, makespan, start_time, input.r, input.c);
    fclose(filew);
    fclose(file);



    //write_to_file("output.txt", "Hello from the utils!\n");
    return 0;
}

