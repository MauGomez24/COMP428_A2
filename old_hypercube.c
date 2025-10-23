#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include "quicksort.h"


int* hypercube_quicksort(int dimension, int *array, int array_size, int rank) 
{
    for (int i = 0; i < dimension - 1; i++) {
        int pivot;

        // choose a pivot using median of three
        if (rank == 0){
            int mid = array_size / 2;
            pivot = median_of_three(array[0], array[mid], array[array_size - 1]);
        }
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // init sub-arrays
        // b1 <= pivot < b2
        int *b1 = malloc(array_size * sizeof(int));
        int *b2 = malloc(array_size * sizeof(int));
        int b1_index = 0;
        int b2_index = 0;

        // populate sub-arrays
        for (int j = 0; j < array_size; j++) {
            if (array[j] <= pivot) {
                b1[b1_index] = array[j];
                b1_index++;
            }
            else {
                b2[b2_index] = array[j];
                b2_index++;
            }
        }

        int neighbour = rank ^ (1 << i);
        int *sent_array, *recv_array;
        int sent_array_size, recv_array_size;

        // check i-th bit
        if (((rank >> i) & 1) == 0) {
            // send b2 and keep b1
            sent_array_size = b2_index;
            sent_array = b2;
        }
        else {
            // send b1 and keep b2
            sent_array_size = b1_index;
            sent_array = b1;
        }

        // now exchange arrays between neighbours
        // first the sizes so that we can properly allocate memory
        MPI_Sendrecv(&sent_array_size, 1, MPI_INT, neighbour, 0, &recv_array_size, 1, MPI_INT, neighbour, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        recv_array = malloc(recv_array_size * sizeof(int));

        // now exchange arrays
        MPI_Sendrecv(sent_array, sent_array_size, MPI_INT, neighbour, 1, recv_array, recv_array_size, MPI_INT, neighbour, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // now we merge the arrays
        int *union_array = malloc((array_size - sent_array_size + recv_array_size) * sizeof(int));
        int union_array_size = 0;

        // C = subsequence received along i-th comm link
        if (((rank >> i) & 1) == 0) {
            // B = B1 union C
            memcpy(union_array, b1, b1_index * sizeof(int));
            memcpy(union_array + b1_index, recv_array, recv_array_size * sizeof(int));
            union_array_size = b1_index + recv_array_size;
        }
        else {
            // B = B2 union C
            memcpy(union_array, b2, b2_index * sizeof(int));
            memcpy(union_array + b2_index, recv_array, recv_array_size * sizeof(int));
            union_array_size = b2_index + recv_array_size;
        }

        // free used memeory
        free(b1);
        free(b2);
        free(recv_array);

        // this is the initial array we passed as a parameter, now poiting at an empty address
        array = union_array;
        array_size = union_array_size;
    }

    // sort the resulting array using sequential quicksort
    quicksort(array, 0, array_size - 1);

    return array;
}