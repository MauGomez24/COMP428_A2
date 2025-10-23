#ifndef HYPERCUBE_H
#define HYPERCUBE_H

void partition_by_pivot(int *array, int array_size, int pivot, int **B1, int *B1_size, int **B2, int *B2_size);
void hypercube_quicksort(int dimension, int **array, int *array_size, int rank);

#endif