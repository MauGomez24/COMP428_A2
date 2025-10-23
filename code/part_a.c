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
    int dimension = atoi(argv[1]);
    int n = atoi(argv[2]); // input_size

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *array = NULL;
    int local_array_size = n / size;

    if (rank == 0) {
        array = generate_array_and_shuffle(n);
    }

    int *local_array = (int *) malloc(local_array_size * sizeof(int));

    MPI_Scatter(array, local_array_size, MPI_INT, local_array, local_array_size, MPI_INT, 0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    hypercube_quicksort(dimension, &local_array, &local_array_size, rank);

    double end_time = MPI_Wtime();

    // gathering data back to process 0
    int *per_process_count = NULL;
    int *array_index_offset;
    if (rank == 0) {
        per_process_count = (int *)malloc(size * sizeof(int));
        array_index_offset = (int *)malloc(size * sizeof(int));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    printf("Rank %d local min = %d, max = %d\n", rank, local_array[0], local_array[local_array_size - 1]);
    MPI_Barrier(MPI_COMM_WORLD);


    MPI_Gather(&local_array_size, 1, MPI_INT, per_process_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        array_index_offset[0] = 0;
        for (int i = 1; i < size; i++) {
            array_index_offset[i] = array_index_offset[i - 1] + per_process_count[i - 1];
        }
    }

    if (rank == 0) {
        printf("Gathering data:\n");
        for (int i = 0; i < size; i++) {
            printf("Process %d: count=%d, offset=%d\n", i, per_process_count[i], array_index_offset[i]);
        }
    }

    // Also print what each process is sending
    printf("Rank %d sending %d elements\n", rank, local_array_size);

    // Ensure all processes finish their sorting and printing before gathering
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Gatherv(local_array, local_array_size, MPI_INT, array, per_process_count, array_index_offset, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            printf("%d ", array[i]);
        }
        exec_time = end_time - start_time;
        printf("\nHypercube quicksort completed in %.6f seconds\n", exec_time);

        free(array);
        free(per_process_count);
        free(array_index_offset);
    }

    free(local_array);
    MPI_Finalize();

    return 0;
}