/**
 * @file psrs_sort.h
 * @brief Function declarations for the optimized PSRS (Parallel Sorting by Regular Sampling) algorithm.
 * 
 * This header defines all helper functions used by the PSRS sorting implementation.
 * The algorithm performs distributed parallel sorting on multiple processes using
 * sampling, pivot selection, in-place partitioning, and an in-place k-way merge.
 * 
 * The functions in this file are invoked by the MPI driver (e.g., part_b.c),
 * while the sequential sorting logic depends on quicksort() defined elsewhere.
 * 
 * COMP 428 - Assignment 2
 * Name: Markus A. Amalfi
 * ID: 40237596
 */

#ifndef PSRS_SORT_H
#define PSRS_SORT_H

/**
 * @brief Comparison function for integer sorting.
 * 
 * Used with the C standard library qsort() and for sampling or pivot operations.
 * Returns the relative ordering between two integer values.
 * 
 * @param a Pointer to the first integer.
 * @param b Pointer to the second integer.
 * @return Negative if *a < *b, positive if *a > *b, zero if equal.
 */
int compare_ints(const void* a, const void* b);

/**
 * @brief Sorts a local data segment using the sequential quicksort algorithm.
 * 
 * Each MPI process calls this function to sort its local subarray of integers
 * before selecting regular samples for the PSRS algorithm.
 * 
 * @param local_data Pointer to the local integer array.
 * @param local_n Number of elements in the local array.
 */
void psrs_local_sort(int* local_data, int local_n);

/**
 * @brief Selects regular samples from a locally sorted array.
 * 
 * Each process chooses evenly spaced samples from its local data to help define
 * global pivot values. These samples are later gathered by the root process.
 * 
 * @param local_data Pointer to the locally sorted array.
 * @param local_n Number of elements in the local array.
 * @param p Total number of processes.
 * @param samples_out Output array of size @p p containing selected samples.
 */
void psrs_select_samples(int* local_data, int local_n, int p, int* samples_out);

/**
 * @brief Determines global pivots from collected samples.
 * 
 * The root process sorts all collected samples and selects @p (p - 1) pivot
 * values to divide the global key space evenly among processes.
 * 
 * @param all_samples Pointer to the array containing all regular samples (size p * p).
 * @param p Total number of processes.
 * @param pivots_out Output array (size p - 1) storing the chosen pivot values.
 */
void psrs_select_pivots(int* all_samples, int p, int* pivots_out);

/**
 * @brief Performs in-place partitioning of a sorted array based on global pivots.
 * 
 * The local array is divided into @p p contiguous partitions. Each partition
 * corresponds to the range of keys destined for a specific process. The function
 * does not allocate new arrays—it simply computes element counts and displacements
 * inside the existing array.
 * 
 * @param local_data Pointer to the local sorted array.
 * @param local_n Number of elements in the local array.
 * @param pivots Array of global pivot values (size p - 1).
 * @param p Total number of processes.
 * @param counts_out Output array (size p) storing element counts for each partition.
 * @param displs_out Output array (size p) storing starting indices for each partition.
 */
void psrs_inplace_partition(int* local_data, int local_n, int* pivots, int p, int* counts_out, int* displs_out);

/**
 * @brief Performs an in-place k-way merge of multiple sorted partitions.
 * 
 * After data exchange, each process owns p sorted sublists.
 * This function merges them into one fully sorted local list using a linear-time
 * multiway merge algorithm. It overwrites the input data buffer in-place.
 * 
 * @param data Pointer to the buffer containing all received sorted partitions.
 * @param displs Array of displacements marking the start index of each partition.
 * @param counts Array of element counts per partition.
 * @param p Number of partitions (equal to the number of processes).
 * @param total_n Total number of elements to merge across all partitions.
 */
void psrs_merge_inplace(int* data, int* displs, int* counts, int p, int total_n);

#endif
