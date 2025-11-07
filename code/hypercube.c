#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include "quicksort.h"

void partition_by_pivot(int *array, int array_size, int pivot, int **B1, int *B1_size, int **B2, int *B2_size) 
{
    *B1 = (int *)malloc(array_size * sizeof(int));
    *B2 = (int *)malloc(array_size * sizeof(int));
    *B1_size = 0;
    *B2_size = 0;

    for (int i = 0; i < array_size; i++) {
        if (array[i] < pivot) {
            (*B1)[(*B1_size)] = array[i];
            (*B1_size)++;
        }
        else {
            (*B2)[(*B2_size)] = array[i];
            (*B2_size)++;
        }
    }
}


void hypercube_quicksort(int dimension, int **array, int *array_size, int rank) 
{
    int *local_array = *array;
    int local_array_size = *array_size;

    // for i between 1 and d do
    for (int i = 1; i <= dimension; i++) {
        // no elements in the array when function is called, but must still communicate
        if (local_array_size == 0) {
            int neighbour = rank ^ (1 << (i - 1));
            int recv_array_size;

            MPI_Sendrecv(&local_array_size, 1, MPI_INT, neighbour, 0, &recv_array_size, 1, MPI_INT, neighbour, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (recv_array_size > 0) {
                local_array = (int *)malloc(recv_array_size * sizeof(int));
                MPI_Sendrecv(NULL, 0, MPI_INT, neighbour, 1, local_array, recv_array_size, MPI_INT, neighbour, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                local_array_size = recv_array_size;
            }
            else {
                    // Both sides are empty - still need to participate in sendrecv
                    MPI_Sendrecv(NULL, 0, MPI_INT, neighbour, 1, NULL, 0, MPI_INT, neighbour, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // local_array_size stays 0
            }
            continue;
        }

        // let's choose a pivot using median of three
        int neighbour = rank ^ (1 << (i-1));
        int pivot;
        // P0 chooses the pivot and then will brodcast it to the others
        if (rank == 0) {
            if (local_array_size == 1 || local_array_size == 2) {
                pivot = local_array[0];
            }
            else {
                int low = 0;
                int high = local_array_size - 1;
                int mid = local_array_size / 2;
                pivot = median_of_three(local_array[low], local_array[mid], local_array[high]);
            }
        }
        else {
            pivot = 0; // default value, will be replaced once the broadcast occurs
        }
        
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    
        // partition input_array into b1 and b2
        // b1: elements <= pivot && b2: elements > pivot
        int *B1, *B2;
        int B1_size, B2_size;
        partition_by_pivot(local_array, local_array_size, pivot, &B1, &B1_size, &B2, &B2_size);

        int *sent_array, *kept_array;
        int sent_array_size, kept_array_size;

        // check i-th bit
        // if zero
        // send b2 to neighbour and receive C from neighbour
        // then set input_array to B1 union C
        if (((rank >> (i - 1)) & 1 ) == 0) {
            sent_array = B2;
            sent_array_size = B2_size;
            kept_array = B1;
            kept_array_size = B1_size;
        }
        // else
        // send b1 to neighbour
        // set input_array to B2 union C
        else {
            sent_array = B1;
            sent_array_size = B1_size;
            kept_array = B2;
            kept_array_size = B2_size;
        }

        // sending and receiving ops
        int recv_array_size;
        MPI_Sendrecv(&sent_array_size, 1, MPI_INT, neighbour, 0, &recv_array_size, 1, MPI_INT, neighbour, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *C = (int *)malloc(recv_array_size * sizeof(int));

        MPI_Sendrecv(sent_array, sent_array_size, MPI_INT, neighbour, 1, C, recv_array_size, MPI_INT, neighbour, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //merging operation
        free(local_array);
        local_array_size = kept_array_size + recv_array_size;
        local_array = (int *)malloc(local_array_size * sizeof(int));

        for (int i = 0; i < kept_array_size; i++) {
            local_array[i] = kept_array[i];
        }
        for (int i = 0; i < recv_array_size; i++) {
            local_array[kept_array_size + i] = C[i];
        }

        free(B1);
        free(B2);
        free(C);
    }
    if (local_array_size > 0) {
        quicksort(local_array, 0, local_array_size - 1);
    }

    *array = local_array;
    *array_size = local_array_size;
}
