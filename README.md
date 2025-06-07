# Job Shop

## Our aproach to the problem
We began by exploring the most direct solutions, we developed a greedy approach ``sequencial_greedy.c``. However, we realized that this approach wouldn’t easly benefit from parallelism due to its linear nature.

We developed another approach utilizing the Branch and Bound algorithm, which explores all possible solutions and finds the most optimal ``sequencial_bb.c``, this takes a long time even for small job schedules. However we can see instant benifits when usig the same algorithm in parallel ``paralelo.c``

## Compile and run the programs

**Sequencial**
```bash
# On Mac
gcc-14 sequencial.c utils.c -o sequencial

# Usage: ./sequencial inputFile outputFile
./sequencial gg03.jss output.txt
```

**Paralelo**
```bash
# On Mac
gcc-14 paralelo.c utils.c -o paralelo

# Usage: ./paralelo inputFile outputFile numberOfThreads
./paralelo gg03.jss output.txt 3
```