Compile Commands
========================================
+ single processor
    - gcc quicksort.c single.c -o single
    - ./single <input_size>

+ hypercube
    - mpicc hypercube.c -o hyper
    - mpirun -np <num_of_processors> hyper <dimension>