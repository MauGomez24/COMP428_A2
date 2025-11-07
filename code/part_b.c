/**
 * @file part_b.c
 * @brief Driver program for comparing sequential Quicksort and PSRS (Parallel Sorting by Regular Sampling).
 * 
 * This optimized version measures and compares sequential quicksort
 * against the PSRS distributed algorithm. It uses in-place partitioning
 * and merging to minimize memory overhead and synchronization latency.
 * 
 * COMP 428 - Assignment 2
 * Name: Markus A. Amalfi
 * ID: 40237596
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "quicksort.h"
#include "psrs_sort.h"

int main(int argc, char *argv[]){

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time, end_time;
    double exec_time_seq = 0.0, exec_time_psrs = 0.0;

    if(argc < 2){
        if(rank == 0){ printf("Usage: mpirun -np <p> ./part_b <input_size>\n"); }
        MPI_Finalize();
        return 0;
    }

    int n = atoi(argv[1]);

    int* array = NULL;
    if(rank == 0){
        array = generate_array_and_shuffle(n);
        int* copy = malloc(n * sizeof(int));
        for(int i = 0; i < n; i++) copy[i] = array[i];

        double seq_start = MPI_Wtime();
        quicksort(copy, 0, n - 1);
        double seq_end = MPI_Wtime();
        exec_time_seq = seq_end - seq_start;

        printf("\n------------------------------------------------\n");
        printf("Sequential and Parallel Quicksort Comparison\n");
        printf("Input size: %d | Processes: %d\n", n, size);
        printf("------------------------------------------------\n");
        printf("Sequential quicksort completed in %.6f seconds\n", exec_time_seq);
        printf("------------------------------------------------\n");

        free(copy);	
    }

    int* send_counts = NULL, *send_displs = NULL; //Send displacements
    int local_n;

    if(rank == 0){
        send_counts = malloc(size * sizeof(int));
        send_displs = malloc(size * sizeof(int));
        int base = n / size, rem = n % size, offset = 0;
		
        for(int i = 0; i < size; i++){
            send_counts[i] = base + (i < rem);
            send_displs[i] = offset;
            offset += send_counts[i];	
        }	
    }

    MPI_Scatter(send_counts, 1, MPI_INT, &local_n, 1, MPI_INT, 0, MPI_COMM_WORLD); //All to All communication
    int* local_array = malloc(local_n * sizeof(int));

    MPI_Scatterv(array, send_counts, send_displs, MPI_INT,
                 local_array, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    // 4.1 Local sort
    psrs_local_sort(local_array, local_n);

    // 4.2 Regular sample selection
    int* samples = malloc(size* sizeof(int));
    psrs_select_samples(local_array, local_n, size, samples);

    // 4.3 Gather samples and choose pivots
    int* all_samples = NULL;
    if(rank == 0){ all_samples = malloc(size * size * sizeof(int)); }
    MPI_Gather(samples, size, MPI_INT, all_samples, size, MPI_INT, 0, MPI_COMM_WORLD);

    int* pivots = malloc((size - 1) * sizeof(int));
    if(rank == 0){ psrs_select_pivots(all_samples, size, pivots); }
    MPI_Bcast(pivots, size - 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 4.4 In-place partition
    int*send_counts_local = calloc(size, sizeof(int));
    int*send_displs_local = calloc(size, sizeof(int));
    psrs_inplace_partition(local_array, local_n, pivots, size, send_counts_local, send_displs_local);

    // 4.5 Exchange partition sizes
    int* recv_counts = malloc(size * sizeof(int));
    MPI_Alltoall(send_counts_local, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int* recv_displs = malloc(size * sizeof(int));
    recv_displs[0] = 0;
    
	for(int i = 1; i < size; i++){
		recv_displs[i] = recv_displs[i - 1] + recv_counts[i - 1];
	}

    int recv_total = recv_displs[size - 1] + recv_counts[size - 1];
    int *recv_buf = malloc(recv_total * sizeof(int));

    // 4.6 Exchange actual partition data
    MPI_Alltoallv(local_array, send_counts_local, send_displs_local, MPI_INT, recv_buf, recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);

    // 4.7 Final in-place merge
    psrs_merge_inplace(recv_buf, recv_displs, recv_counts, size, recv_total);

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    exec_time_psrs = end_time - start_time;

    if(rank == 0){
        printf("PSRS sorting completed in %.6f seconds\n", exec_time_psrs);
        printf("------------------------------------------------\n");
        printf("Speedup over sequential quicksort: %.2fx\n", exec_time_seq / exec_time_psrs);
        printf("------------------------------------------------\n");
	}

    free(local_array);
    free(samples);
    free(pivots);
    free(send_counts_local);
    free(send_displs_local);
    free(recv_counts);
    free(recv_displs);
    free(recv_buf);

    if(rank == 0){
        free(array);
        free(send_counts);
        free(send_displs);
        free(all_samples);
	}

    MPI_Finalize();
    return 0;

}