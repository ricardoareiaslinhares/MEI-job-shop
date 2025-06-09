# Job Shop Scheduling

## Branch and Bound implementation

The file ``sequencial_bb.c`` was the first implementation of this algorithm. When we started on the parallel version and realized that we had to partition the problem into individual work units, we also implemented the same partitioning for the sequential code. Hence, for purposes of comparison with the parallel code, consider the ``sequencial_bb2.c`` file.


## Compile and run the programs

**Sequencial**
```bash
# On Mac
gcc-14 sequencial_bb2.c utils.c -o sequencial

# Usage: ./sequencial_bb2 inputFile outputFile
./sequencial_bb2 gg03.jss outputS.txt
```

**Paralelo**
```bash
# On Mac
gcc-14 paralelo.c utils.c -o paralelo

# Usage: ./paralelo inputFile outputFile numberOfThreads
./paralelo gg03.jss outputP.txt 8
```