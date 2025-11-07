Compile Commands
========================================
+ single processor
    - gcc quicksort.c single.c -o single
    - ./single <input_size>

+ hypercube
    - mpicc quicksort.c hypercube.c part_a.c  -o hyper -lm
    - mpirun -np <num_of_processors> hyper <input_size>