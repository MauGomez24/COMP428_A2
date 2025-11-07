Compile Commands
========================================
+ single processor
    - gcc quicksort.c single.c -o single
    - ./single <input_size>

+ hypercube
    - mpicc quicksort.c hypercube.c part_a.c  -o hyper -lm
    - mpirun -np <num_of_processors> --hostfile /media/pkg/apini-scripts/apini_hostfile hyper <input_size>

+ psrs
    -mpicc -O3 -o part_b part_b.c psrs_sort.c quicksort.c -lm
    -mpirun -np <num_of_processors> --hostfile /media/pkg/apini-scripts/apini_hostfile ./part_b <number of inputs to generate>