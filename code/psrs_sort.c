/**
 * @file psrs_sort.c
 * @brief Optimized PSRS helper routines with in-place partitioning and merge.
 * 
 * Implements the core computation of PSRS without MPI calls, using in-place
 * partitioning and k-way merging for efficiency. This version minimizes memory
 * copying and alloc/free overhead compared to the baseline implementation.
 * 
 * COMP 428 - Assignment 2
 * Name: Markus A. Amalfi
 * ID: 40237596
 * 
 * Name: Mauricio Gomez Iglesias
 * ID: 40244800
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//------------------------------------------------------------//
// Comparator for qsort
//------------------------------------------------------------//
int compare_ints(const void* a, const void* b){
	
	int x = *(int*)a;
	int y = *(int*)b;
	return (x > y) - (x < y);

}

//------------------------------------------------------------//
// Step 1: Local sort using quicksort()
//------------------------------------------------------------//
void psrs_local_sort(int* local_data, int local_n){
	
	extern void quicksort(int array[], int low, int high);
	quicksort(local_data, 0, local_n - 1);

}

//------------------------------------------------------------//
// Step 2: Pick regular samples (balanced spacing)
//------------------------------------------------------------//
void psrs_select_samples(int* local_data, int local_n, int p, int* samples_out){
	
	for(int i = 0; i < p; i++){
		
		int idx = (int)(((double)i * local_n) / p + 0.5);
		if(idx >= local_n) idx = local_n - 1;
		samples_out[i] = local_data[idx];
	
	}

}

//------------------------------------------------------------//
// Step 3: Choose pivots more evenly (rank 0 only)
//------------------------------------------------------------//
void psrs_select_pivots(int* all_samples, int p, int* pivots_out){
	
	int total = p * p;
	extern void quicksort(int array[], int low, int high);
	quicksort(all_samples, 0, total - 1);

	//Pivot Selection
	for(int i = 0; i < p - 1; i++){
	
		pivots_out[i] = all_samples[(i + 1) * p - 1];
	
	}
	
}

//------------------------------------------------------------//
// Step 4: In-place partitioning (no malloc per partition)
//------------------------------------------------------------//
void psrs_inplace_partition(int* local_data, int local_n, int* pivots, int p, int* counts_out, int* displs_out){

	int pivot_idx = 0;
	int start = 0;
	
	for(int i = 0; i < p - 1; i++){
		
		int end = start;
		while(end < local_n && local_data[end] <= pivots[i]){ end++; } //Iterate until end is reached
		counts_out[i] = end - start;
		displs_out[i] = start;
		start = end;
	}
	
	counts_out[p - 1] = local_n - start;
	displs_out[p - 1] = start;
	
}

//------------------------------------------------------------//
// Step 5: K-way merge (in-place variant)
//------------------------------------------------------------//
void psrs_merge_inplace(int *data, int *displs, int *counts, int p, int total_n){

	int* indices = calloc(p, sizeof(int));
	int* output = malloc(total_n * sizeof(int));

	for(int k = 0; k < total_n; k++){ //Perform mergesort loop here
		
		int min_val = INT_MAX;
		int min_i = -1;

		for(int i = 0; i < p; i++){
			
			int offset = displs[i] + indices[i];
			
			if(indices[i] < counts[i] && data[offset] < min_val){
				
				min_val = data[offset];
				min_i = i;
			
			}
			
		}
		
		output[k] = min_val;
		indices[min_i]++;
		
	}

	for(int i = 0; i < total_n; i++){ data[i] = output[i]; } //Gather data
	free(indices);
	free(output);
	
}
