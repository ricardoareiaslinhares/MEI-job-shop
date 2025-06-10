# Pseudocode of the branch and Bound algorithm

void branch_bound(const InputData *input, const State *current_state, int stop_depth) {
    if (current_state->depth == stop_depth) { ... atualizar solução ...; return; }
    for (cada job com operações por fazer) {
        calcular máquina, duração, início e fim;
        se (fim >= min_makespan) continuar; // poda
        criar nova cópia do estado;
        atualizar nova cópia com esta operação;
        branch_bound(input, &nova_copia, stop_depth); // chamada recursiva
    }
}

```c

typedef struct {
    int machines[MAX][MAX];
    int times[MAX][MAX];
    int r, c; // rows and columns
} InputData;

typedef struct {
    int depth;
    int job_free[MAX];
    int machine_free[MAX];
    int job_op[MAX];
    int makespan;
    int start_time[MAX][MAX];
} State;

int min_makespan = INT_MAX; // variável global
int stop_depth = input.r * input.c

void branch_bound(InputData *input, State *current_state, int stop_depth ) {
    if (current_state.depth == stop_depth) {
        Condição de saída e atualiza o min_makespan
        }
    for (cada job com operações por fazer) {
        if (não tiver mais operações por fazer) continue;

        definir número da operação, máquina, duraçao atual, ínicio e fim da operação;

        if (fim >= min_makespan) continue / não precisa de continuar mais neste branch;

        criar um novo State new_state e povoar

       branch_bound(input, &new_state, stop_depth); 
    }
}

```


Partitioning function

```c
void travel_and_generate_work(InputData *input, State *current_state, int stop_depth, State *array_work_units, int *count ) {
    if (current_state.depth == stop_depth) {
        Guarda o estado atual em worn_units, na posição count
        }
    for (cada job com operações por fazer) {
        ... // mesma implementação de branch_bound

       travel_and_generate_work(input, &new_state, stop_depth, work_units, count); 
    }
}
```
