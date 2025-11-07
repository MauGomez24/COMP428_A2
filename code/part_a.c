/**
 * COMP 428 - Assignment 2
 * Name: Markus A. Amalfi
 * ID: 40237596
 * 
 * Name: Mauricio Gomez Iglesias
 * ID: 40244800
 * 
 * This file contains the main function that will call the hypercube_quicksort algorithm
 */

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
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // run check: num of processors must be a power of 2
    int dimension = (int)log2(size);
    if ((1 << dimension) != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Number of processors must be a power of 2\n");
        }
        MPI_Finalize();
        return 1;
    }

    // set the input size
    int n = atoi(argv[1]);

    // initialize local variables
    int *array = NULL;
    int local_array_size = n / size;

    // Generate and shuffle array on rank 0
    if (rank == 0) {
        array = generate_array_and_shuffle(n);
        printf("Original array size: %d\n", n);
        printf("Number of processors: %d (dimension: %d)\n", size, dimension);

        // // prints the sorted array 
        // printf("Unsorted array:\n");
        // for (int i = 0; i < n; i++) {
        //     printf("%d ", array[i]);
        // }
        // printf("\n\n");
    }


    int *local_array = (int *)malloc(local_array_size * sizeof(int));

    // prepare the data to be sent to each process through Scatterv
    int *send_array_size = NULL;
    int *send_array_offset = NULL;
    
    if (rank == 0) {
        send_array_size = (int *)malloc(size * sizeof(int));
        send_array_offset = (int *)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; i++) {
            send_array_size[i] = n / size;
            send_array_offset[i] = offset;
            offset += send_array_size[i];
        }
    }

    // Scatter the data
    MPI_Scatterv(array, send_array_size, send_array_offset, MPI_INT, 
                 local_array, local_array_size, MPI_INT, 
                 0, MPI_COMM_WORLD);

    // synchronize all processes and start the counter
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // calling hypercube_quicksort
    hypercube_quicksort(dimension, &local_array, &local_array_size, rank);

    // synchronize all processes again and stop the time
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    exec_time = end_time - start_time;

    // prepare data to be gathered by P0
    int *received_array_size = NULL;
    int *received_array_offset = NULL;
    
    if (rank == 0) {
        received_array_size = (int *)malloc(size * sizeof(int));
        received_array_offset = (int *)malloc(size * sizeof(int));
    }

    // get the size of each sorted sub_array
    MPI_Gather(&local_array_size, 1, MPI_INT, received_array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int offset = 0;
        for (int i = 0; i < size; i++) {
            received_array_offset[i] = offset;
            offset += received_array_size[i];
        }
    }

    // now we gather the subarrays
    MPI_Gatherv(local_array, local_array_size, MPI_INT,
                array, received_array_size, received_array_offset, MPI_INT,
                0, MPI_COMM_WORLD);

    // P0 will print the results
    if (rank == 0) {
        printf("\nExecution time: %f seconds\n", exec_time);

        // // prints sorted array -- here for a sanity check
        // printf("Sorted array:\n");
        // for (int i = 0; i < n; i++) {
        //     printf("%d ", array[i]);
        // }
        // printf("\n\n");
        
        // free used memory
        free(array);
        free(send_array_size);
        free(send_array_offset);
        free(received_array_size);
        free(received_array_offset);
    }

    free(local_array);

    MPI_Finalize();
    return 0;
}