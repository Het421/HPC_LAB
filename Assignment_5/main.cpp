// #include <cstdio>
// #include <cstdlib>
// #include <cstring>
// #include <omp.h>
// #include "init.h"
// #include "utils.h"

// // Global variables
// int GRID_X, GRID_Y, NX, NY;
// long long NUM_Points;
// long long Maxiter = 10;
// double dx, dy;

// int main()
// {
//     int Nx_values[3] = {250, 500, 1000};
//     int Ny_values[3] = {100, 200, 400};

//     // Safe values for Lab PC (use 1e9 only on HPC)
//     long long particle_counts[5] = {
//         100,
//         10000,
//         1000000,
//         10000000,
//         100000000
//     };

//     for (int config = 0; config < 3; config++)
//     {
//         NX = Nx_values[config];
//         NY = Ny_values[config];

//         GRID_X = NX + 1;
//         GRID_Y = NY + 1;

//         dx = 1.0 / NX;
//         dy = 1.0 / NY;

//         printf("\n=============================\n");
//         printf("Configuration %d\n", config + 1);
//         printf("NX=%d NY=%d\n", NX, NY);
//         printf("=============================\n");

//         // Output file for plotting
//         char filename[50];
//         sprintf(filename, "output_config_parallel_defferred_%d.txt", config + 1);
//         FILE *fp = fopen(filename, "w");

//         fprintf(fp, "Particles TotalTime\n");

//         for (int pc = 0; pc < 5; pc++)
//         {
//             NUM_Points = particle_counts[pc];

//             printf("\nParticles = %lld\n", NUM_Points);

//             Points *points = new Points[NUM_Points];
//             double *mesh_value = new double[GRID_X * GRID_Y];

//             // ✅ Initialize ONCE
//             initializepoints(points);

//             double total_interp = 0.0;
//             double total_mover = 0.0;

//             for (int iter = 0; iter < Maxiter; iter++)
//             {
//                 memset(mesh_value, 0, GRID_X * GRID_Y * sizeof(double));

//                 double t1 = omp_get_wtime();
//                 interpolation(mesh_value, points);
//                 double t2 = omp_get_wtime();

//                 double t3 = omp_get_wtime();
//                 mover_parallel_deferred(points, dx, dy);
//                 double t4 = omp_get_wtime();

//                 total_interp += (t2 - t1);
//                 total_mover += (t4 - t3);
//             }

//             double total = total_interp + total_mover;

//             printf("Total Time = %lf\n", total);

//             // Save for plotting
//             fprintf(fp, "%lld %lf\n", NUM_Points, total);

//             delete[] points;
//             delete[] mesh_value;
//         }

//         fclose(fp);
//     }

//     return 0;
// }


//  Experiment 2

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>
#include "init.h"
#include "utils.h"

// Global variables
int GRID_X, GRID_Y, NX, NY;
long long NUM_Points;
long long Maxiter = 10;
double dx, dy;

int main()
{
    int Nx_values[3] = {250, 500, 1000};
    int Ny_values[3] = {100, 200, 400};

    int thread_counts[4] = {2, 4, 8, 16};

    NUM_Points = 14000000;

    for (int config = 0; config < 3; config++)
    {
        NX = Nx_values[config];
        NY = Ny_values[config];

        GRID_X = NX + 1;
        GRID_Y = NY + 1;

        dx = 1.0 / NX;
        dy = 1.0 / NY;

        printf("\n=============================\n");
        printf("Config %d | NX=%d NY=%d\n", config+1, NX, NY);
        printf("=============================\n");

        // ============================
        // FILES
        // ============================
        char file_im[50], file_def[50];
        sprintf(file_im, "parallel_immediate_config_%d.txt", config+1);
        // sprintf(file_def, "parallel_deferred_config_%d.txt", config+1);

        FILE *fp_im = fopen(file_im, "w");
        // FILE *fp_def = fopen(file_def, "w");

        fprintf(fp_im, "Threads TotalTime\n");
        // fprintf(fp_def, "Threads TotalTime\n");

        // ============================
        // PHASE 1: IMMEDIATE
        // ============================
        printf("\n--- Immediate Mover ---\n");

        for (int t = 0; t < 4; t++)
        {
            int threads = thread_counts[t];
            omp_set_num_threads(threads);

            printf("Threads = %d\n", threads);

            Points *points = new Points[NUM_Points];
            double *mesh_value = new double[GRID_X * GRID_Y];

            initializepoints(points);

            double total_time = 0.0;

            for (int iter = 0; iter < Maxiter; iter++)
            {
                memset(mesh_value, 0, GRID_X * GRID_Y * sizeof(double));

                double t1 = omp_get_wtime();
                interpolation(mesh_value, points);

                mover_parallel_immediate(points, dx, dy);
                double t2 = omp_get_wtime();

                total_time += (t2 - t1);
            }

            printf("Time = %lf\n", total_time);
            fprintf(fp_im, "%d %lf\n", threads, total_time);

            delete[] points;
            delete[] mesh_value;
        }

        // ============================
        // PHASE 2: DEFERRED
        // ============================
        // printf("\n--- Deferred Mover ---\n");

        // for (int t = 0; t < 4; t++)
        // {
        //     int threads = thread_counts[t];
        //     omp_set_num_threads(threads);

        //     printf("Threads = %d\n", threads);

        //     Points *points = new Points[NUM_Points];
        //     double *mesh_value = new double[GRID_X * GRID_Y];

        //     initializepoints(points);

        //     double total_time = 0.0;

        //     for (int iter = 0; iter < Maxiter; iter++)
        //     {
        //         memset(mesh_value, 0, GRID_X * GRID_Y * sizeof(double));

        //         double t1 = omp_get_wtime();
        //         interpolation(mesh_value, points);

        //         mover_parallel_deferred(points, dx, dy);
        //         double t2 = omp_get_wtime();

        //         total_time += (t2 - t1);
        //     }

        //     printf("Time = %lf\n", total_time);
        //     // fprintf(fp_def, "%d %lf\n", threads, total_time);

        //     delete[] points;
        //     delete[] mesh_value;
        // }

        // fclose(fp_im);
        // // fclose(fp_def);
    }

    return 0;
}