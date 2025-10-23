#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// auxiliary function for swapping two integers
void swap(int *x, int *y)
{
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

// auxiliary function to find the median of the array
int median_of_three(int low, int mid, int high) 
{
    if ((mid <= low && low <= high) || (high <= low && low <= mid))
        return low;
    else if ((low <= mid && mid <= high) || (high <= mid && mid <= low))
        return mid;
    else
        return high;
}

// partition function for quicksort
int partition(int array[], int low, int high)
{
    int mid = low + (high - low) / 2;
    int pivot = median_of_three(array[low], array[mid], array[high]);
    int i = low - 1;

    if (pivot == array[mid])
        swap(&array[mid], &array[high]);
    else if (pivot == array[low])
        swap(&array[low], &array[high]);

    for (int j = low; j < high; j++) {
        if (array[j] < pivot) {
            i++;
            swap(&array[i], &array[j]);
        }
    }

    swap(&array[i + 1], &array[high]);
    return i + 1;
}

// sequential quicksort for final local sorting
void quicksort(int array[], int low, int high)
{
    if (low < high) {
        int partition_index = partition(array, low, high);
        quicksort(array, low, partition_index - 1);
        quicksort(array, partition_index + 1, high);
    }
}

// partition array into two parts: elements < pivot and elements >= pivot
void partition_by_pivot(int *array, int size, int pivot, 
                        int **B1, int *size1, int **B2, int *size2)
{
    *B1 = (int *)malloc(size * sizeof(int));
    *B2 = (int *)malloc(size * sizeof(int));
    *size1 = 0;
    *size2 = 0;

    for (int i = 0; i < size; i++) {
        if (array[i] < pivot) {
            (*B1)[(*size1)++] = array[i];
        } else {
            (*B2)[(*size2)++] = array[i];
        }
    }
}

// hypercube quicksort main procedure
void hypercube_quicksort(int **B, int *n, int id, int d)
{
    int *local_array = *B;
    int local_size = *n;

    // iterate through each dimension of the hypercube (1 to d)
    for (int i = 1; i <= d; i++) {
        if (local_size == 0) {
            // if no elements, still participate in communication
            int partner = id ^ (1 << (i - 1));
            int recv_size;
            
            MPI_Sendrecv(&local_size, 1, MPI_INT, partner, 0,
                        &recv_size, 1, MPI_INT, partner, 0,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if (recv_size > 0) {
                local_array = (int *)malloc(recv_size * sizeof(int));
                MPI_Recv(local_array, recv_size, MPI_INT, partner, 1,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                local_size = recv_size;
            }
            continue;
        }

        // choose pivot (using median of local array)
        int pivot;
        if (local_size == 1) {
            pivot = local_array[0];
        } else if (local_size == 2) {
            pivot = (local_array[0] + local_array[1]) / 2;
        } else {
            int low = 0, mid = local_size / 2, high = local_size - 1;
            pivot = median_of_three(local_array[low], local_array[mid], 
                                   local_array[high]);
        }

        // broadcast pivot across the current dimension subcube
        // all processes in the same subcube need the same pivot
        MPI_Allreduce(MPI_IN_PLACE, &pivot, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

        // partition local array into B1 (< pivot) and B2 (>= pivot)
        int *B1, *B2;
        int size1, size2;
        partition_by_pivot(local_array, local_size, pivot, &B1, &size1, &B2, &size2);

        // determine partner process along i-th communication link
        // since i goes from 1 to d, we use (i-1) for bit position
        int partner = id ^ (1 << (i - 1));
        int i_bit = (id >> (i - 1)) & 1;

        int *send_array, *keep_array;
        int send_size, keep_size;

        if (i_bit == 0) {
            // send B2 (larger elements), keep B1 (smaller elements)
            send_array = B2;
            send_size = size2;
            keep_array = B1;
            keep_size = size1;
        } else {
            // send B1 (smaller elements), keep B2 (larger elements)
            send_array = B1;
            send_size = size1;
            keep_array = B2;
            keep_size = size2;
        }

        // exchange sizes first
        int recv_size;
        MPI_Sendrecv(&send_size, 1, MPI_INT, partner, 0,
                     &recv_size, 1, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // allocate buffer for received data
        int *C = (int *)malloc(recv_size * sizeof(int));

        // exchange actual data
        MPI_Sendrecv(send_array, send_size, MPI_INT, partner, 1,
                     C, recv_size, MPI_INT, partner, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // merge kept partition with received data
        free(local_array);
        local_size = keep_size + recv_size;
        local_array = (int *)malloc(local_size * sizeof(int));

        for (int j = 0; j < keep_size; j++) {
            local_array[j] = keep_array[j];
        }
        for (int j = 0; j < recv_size; j++) {
            local_array[keep_size + j] = C[j];
        }

        // cleanup
        free(B1);
        free(B2);
        free(C);
    }

    // final local sequential quicksort
    if (local_size > 0) {
        quicksort(local_array, 0, local_size - 1);
    }

    *B = local_array;
    *n = local_size;
}

// generate and shuffle array
int* generate_array_and_shuffle(int input_size)
{
    int *array = (int *)malloc(input_size * sizeof(int));
    for (int i = 0; i < input_size; i++) {
        array[i] = i;
    }

    srand(time(NULL));
    for (int i = input_size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&array[i], &array[j]);
    }

    return array;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // verify that number of processes is a power of 2
    int d = (int)log2(size);
    if ((1 << d) != size) {
        if (rank == 0) {
            printf("Error: Number of processes must be a power of 2\n");
        }
        MPI_Finalize();
        return 1;
    }

    int n = 1000; // total array size
    if (argc > 1) {
        n = atoi(argv[1]);
    }

    int *array = NULL;
    int local_n = n / size;

    // root process generates the array
    if (rank == 0) {
        array = generate_array_and_shuffle(n);
        printf("Generated array of size %d for %d processes\n", n, size);
    }

    // allocate local array
    int *local_array = (int *)malloc(local_n * sizeof(int));

    // distribute data to all processes
    MPI_Scatter(array, local_n, MPI_INT, local_array, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    // perform hypercube quicksort
    hypercube_quicksort(&local_array, &local_n, rank, d);

    double end_time = MPI_Wtime();

    // gather sorted data back to root
    int *recv_counts = NULL;
    int *displs = NULL;
    if (rank == 0) {
        recv_counts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
    }

    MPI_Gather(&local_n, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < size; i++) {
            displs[i] = displs[i-1] + recv_counts[i-1];
        }
    }

    MPI_Gatherv(local_array, local_n, MPI_INT, array, recv_counts, displs, 
                MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Hypercube quicksort completed in %.6f seconds\n", end_time - start_time);
        
        // verify sorting
        int sorted = 1;
        int total_elements = 0;
        for (int i = 0; i < size; i++) {
            total_elements += recv_counts[i];
        }
        
        for (int i = 1; i < total_elements; i++) {
            if (array[i] < array[i-1]) {
                sorted = 0;
                break;
            }
        }
        printf("Array is %s\n", sorted ? "correctly sorted" : "NOT sorted");
        
        free(array);
        free(recv_counts);
        free(displs);
    }

    free(local_array);
    MPI_Finalize();
    return 0;
}