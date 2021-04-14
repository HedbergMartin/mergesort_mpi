# mergesort_mpi
Parallel implementation of mergesort using MPI.

## Compile
mpicc -o mergesort mergesort.c

## Execution
mpirun -np [number of cores] ./mergesort [number of size duplications] [inital array size]

### Abisko
Size increas experiment: squeue job_msSizes

Core increas experiment: squeue job_msCores