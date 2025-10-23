/**
 * COMP 428 - Assignment 2
 * Name: Mauricio Gomez Iglesias
 * ID: 40244800
 * 
 * This file contains a the implementation for sorting an array of size n using the quicksort algorithm on a single processing unit
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "quicksort.h"

int main(int argc, char *argv[])
{
    // set the random seed and get the input size from command line
    srand(time(NULL));
    int input_size = atoi(argv[1]);

    // generating array of random ints
    int n = input_size;
    int *array = generate_array_and_shuffle(n);

    //print unsorted array first
    printf("Unsorted array: \n");
    for (int i = 0; i < n; i++) {
        printf("%d ", array[i]);
    }

    // start measuring program execution time
    clock_t start, end;
    start = clock();

    // call quicksort
    quicksort(array, 0, n - 1);

    // finish time measurement
    end = clock();
    double exec_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    // print sorted array and exec time
    printf("\n\nSorted array: \n");
    for (int i = 0; i < n; i++) {
        printf("%d ", array[i]);
    }

    free(array);

    printf("\n");
    printf ("\nSorting took %f seconds \n", exec_time);

    return 0;
}