#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Serial interpolation 
// void interpolation(double *mesh_value, Points *points) {

//     const double inv_dx = 1.0 / dx;
//     const double inv_dy = 1.0 / dy;

//     for (int p = 0; p < NUM_Points; p++) {

//         double x = points[p].x;
//         double y = points[p].y;

//         int i = (int)(x * inv_dx);
//         int j = (int)(y * inv_dy);

//         if (i >= NX) i = NX - 1;
//         if (j >= NY) j = NY - 1;

//         double x_i = i * dx;
//         double y_j = j * dy;

//         double dx_local = (x - x_i) * inv_dx;
//         double dy_local = (y - y_j) * inv_dy;

//         double one_minus_dx = 1.0 - dx_local;
//         double one_minus_dy = 1.0 - dy_local;

//         double w00 = one_minus_dx * one_minus_dy;
//         double w10 = dx_local * one_minus_dy;
//         double w01 = one_minus_dx * dy_local;
//         double w11 = dx_local * dy_local;

//         int base = j * GRID_X + i;

//         mesh_value[base]                 += w00;
//         mesh_value[base + 1]             += w10;
//         mesh_value[base + GRID_X]        += w01;
//         mesh_value[base + GRID_X + 1]    += w11;
//     }
// }
// parallel interpolation using OpenMP

void interpolation(double *mesh_value, Points *points) {

    const double inv_dx = 1.0 / dx;
    const double inv_dy = 1.0 / dy;

    #pragma omp parallel
    {
        double *local_mesh = (double *) calloc(GRID_X * GRID_Y, sizeof(double));

        #pragma omp for
        for (int p = 0; p < NUM_Points; p++) {

            double x = points[p].x;
            double y = points[p].y;

            int i = (int)(x * inv_dx);
            int j = (int)(y * inv_dy);

            if (i >= NX) i = NX - 1;
            if (j >= NY) j = NY - 1;

            double x_i = i * dx;
            double y_j = j * dy;

            double dx_local = (x - x_i) * inv_dx;
            double dy_local = (y - y_j) * inv_dy;

            double one_minus_dx = 1.0 - dx_local;
            double one_minus_dy = 1.0 - dy_local;

            double w00 = one_minus_dx * one_minus_dy;
            double w10 = dx_local * one_minus_dy;
            double w01 = one_minus_dx * dy_local;
            double w11 = dx_local * dy_local;

            int base = j * GRID_X + i;

            local_mesh[base] += w00;
            local_mesh[base + 1] += w10;
            local_mesh[base + GRID_X] += w01;
            local_mesh[base + GRID_X + 1] += w11;
        }

        // Combine results
        #pragma omp critical
        {
            for (int i = 0; i < GRID_X * GRID_Y; i++) {
                mesh_value[i] += local_mesh[i];
            }
        }

        free(local_mesh);
    }
}

void read_points(FILE *file, Points *points) {
    for (int i = 0; i < NUM_Points; i++) {
        fread(&points[i].x, sizeof(double), 1, file);
        fread(&points[i].y, sizeof(double), 1, file);
    }
}
// Write mesh to file
void save_mesh(double *mesh_value) {

    FILE *fd = fopen("output_e_parallel", "w");
    if (!fd) {
        printf("Error creating Mesh.out\n");
        exit(1);
    }

    for (int i = 0; i < GRID_Y; i++) {
        for (int j = 0; j < GRID_X; j++) {
            fprintf(fd, "%lf ", mesh_value[i * GRID_X + j]);
        }
        fprintf(fd, "\n");
    }

    fclose(fd);
}
