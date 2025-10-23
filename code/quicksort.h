#ifndef QUICKSORT_H
#define QUICKSORT_H

void swap(int *x, int *y);
int median_of_three(int low, int mid, int high);
int partition(int array[], int low, int high);
void quicksort(int array[], int low, int high);
int* generate_array_and_shuffle(int input_size);

#endif