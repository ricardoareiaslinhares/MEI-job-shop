#ifndef UTILS_H
#define UTILS_H
#define MAX 50

typedef struct {
    int machines[MAX][MAX];
    int times[MAX][MAX];
    int r, c;
} InputData;

InputData parse_file(FILE *file);
void print_input_data(InputData data);
void write_to_file(FILE *file, int makespan, int start_time[MAX][MAX], int num_jobs, int num_machines);
int file_check(FILE *file, FILE *filew);

#endif
