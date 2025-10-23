/**
 * COMP 428 - Assignment 2
 * Name: Mauricio Gomez Iglesias
 * ID: 40244800
 * 
 * This file contains a standard implementation of the quicksort algorithm that will be called
 * by the different programs for the assignment
 */

#include <stdlib.h>
#include <time.h>

// auxiliary function for swapping two integers
void swap(int *x, int *y)
{
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

// auxiliary function to find the median of the array
// useful for hypercube implementation
int median_of_three(int low, int mid, int high) 
{
    if ((mid <= low && low <= high) || (high <= low && low <= mid))
        return low;
    else if ((low <= mid && mid <= high) || (high <= mid && mid <= low))
        return mid;
    else
        return high;
}

//
int partition(int array[], int low, int high)
{
    int mid = low + (high - low) / 2;
    int pivot = median_of_three(array[low], array[mid], array[high]);
    int i = low - 1;

    // now place the pivot at the end of the array if it is the middle value or the first one
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

// the quicksort function itself
void quicksort(int array[], int low, int high)
{
    if (low < high) {
        int partition_index = partition(array, low, high);
        quicksort(array, low, partition_index - 1);
        quicksort(array, partition_index + 1, high);
    }
}

// finally a function to initialize our test array
int* generate_array_and_shuffle(int input_size)
{
    // allocate memory for the array
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