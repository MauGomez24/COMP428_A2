#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include "quicksort.h"
#include "hypercube.h"


int main (int argc, char *argv[]) 
{
    MPI_Init(&argc, &argv);

    int rank, size;
    double exec_time;
    // init dimension and input_size vars
    // get dimension from size (d = log size)
    int dimension = atoi(argv[1]);
    int n = atoi(argv[2]); // input_size

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *array = NULL;
    int local_array_size = n / size;

    if (rank == 0) {
        array = generate_array_and_shuffle(n);
    }

    
    return 0;
}