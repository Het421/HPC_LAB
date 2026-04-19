

// Parallelized version of main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h> //  ADD THIS

#include "init.h"
#include "utils.h"

// Global variables
int GRID_X, GRID_Y, NX, NY;
int NUM_Points, Maxiter;
double dx, dy;

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    int thread_list[] = {1, 2, 4, 8, 16}; //  ADD
    int num_tests = 5;                    //  ADD

    for (int tt = 0; tt < num_tests; tt++)
    { //  ADD LOOP

        int threads = thread_list[tt];
        omp_set_num_threads(threads); //  SET THREADS

        printf("\n==============================\n");
        printf("Running with %d threads\n", threads);
        printf("==============================\n");

        // Open binary file for reading (MOVE INSIDE LOOP)
        FILE *file = fopen(argv[1], "rb");
        if (!file)
        {
            printf("Error opening input file\n");
            exit(1);
        }

        // Read grid dimensions
        if (fread(&NX, sizeof(int), 1, file) != 1)
        {
            printf("Error reading NX\n");
            exit(1);
        }
        if (fread(&NY, sizeof(int), 1, file) != 1)
        {
            printf("Error reading NX\n");
            exit(1);
        }

        // Read number of Points and max iterations
        fread(&NUM_Points, sizeof(int), 1, file);
        fread(&Maxiter, sizeof(int), 1, file);

        GRID_X = NX + 1;
        GRID_Y = NY + 1;
        dx = 1.0 / NX;
        dy = 1.0 / NY;

        double *mesh_value = (double *)calloc(GRID_X * GRID_Y, sizeof(double));
        Points *points = (Points *)calloc(NUM_Points, sizeof(Points));

        double total_int_time = 0.0;
        double total_norm_time = 0.0;
        double total_move_time = 0.0;
        double total_denorm_time = 0.0;

        // Read scattered points (INSIDE LOOP)
        read_points(file, points);

        fclose(file);

        for (int iter = 0; iter < Maxiter; iter++)
        {

            double t0 = omp_get_wtime();

            interpolation(mesh_value, points);

            double t1 = omp_get_wtime();

            normalization(mesh_value);

            double t3 = omp_get_wtime();

            mover(mesh_value, points);

            double t4 = omp_get_wtime();

            denormalization(mesh_value);

            double t5 = omp_get_wtime();

            total_int_time += (double)(t1 - t0) ;
            total_norm_time += (double)(t3 - t1) ;
            total_move_time += (double)(t4 - t3);
            total_denorm_time += (double)(t5 - t4);
        }

        printf("Total Interpolation Time = %lf seconds\n", total_int_time);
        printf("Total Normalization Time = %lf seconds\n", total_norm_time);
        printf("Total Mover Time = %lf seconds\n", total_move_time);
        printf("Total Denormalization Time = %lf seconds\n", total_denorm_time);
        printf("Total Algorithm Time = %lf seconds\n",
               total_int_time + total_norm_time + total_move_time + total_denorm_time);
        printf("Total Number of Voids = %lld\n", void_count(points));

        // Save only once (optional: only for 1 thread)
        if (threads == 1)
        {
            save_mesh(mesh_value);
        }

        free(mesh_value);
        free(points);
    }

    return 0;
}