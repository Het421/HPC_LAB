// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "utils.h"

// double min_val, max_val;

// // interpolation 
// void interpolation(double *mesh_value, Points *points) {

//     // Reset mesh
//     memset(mesh_value, 0, sizeof(double) * GRID_X * GRID_Y);

//     for (int p = 0; p < NUM_Points; p++) {

//         if (points[p].is_void) continue;

//         double x = points[p].x;
//         double y = points[p].y;
//         //double f = 1.0; // Assuming a constant value for interpolation, can be modified as needed

//         int i = (int)(x / dx);
//         int j = (int)(y / dy);

//         // Clamp (important for boundary safety)
//         if (i >= NX) i = NX - 1;
//         if (j >= NY) j = NY - 1;

//         double Xi = i * dx;
//         double Yj = j * dy;

//         double lx = x - Xi;
//         double ly = y - Yj;

//         // Weights
//         double w00 = (dx - lx) * (dy - ly);
//         double w10 = lx * (dy - ly);
//         double w01 = (dx - lx) * ly;
//         double w11 = lx * ly;

//         int idx = j * GRID_X + i;

//         mesh_value[idx]                 += w00;
//         mesh_value[idx + 1]             += w10;
//         mesh_value[idx + GRID_X]        += w01;
//         mesh_value[idx + GRID_X + 1]    += w11;
//     }
// }

// void normalization(double *mesh_value) {

//     min_val = 1e18;
//     max_val = -1e18;

//     int size = GRID_X * GRID_Y;

//     for (int i = 0; i < size; i++) {
//         if (mesh_value[i] < min_val) min_val = mesh_value[i];
//         if (mesh_value[i] > max_val) max_val = mesh_value[i];
//     }

//     double range = max_val - min_val;

//     if (range == 0.0) return;

//     for (int i = 0; i < size; i++) {
//         mesh_value[i] = 2.0 * (mesh_value[i] - min_val) / range - 1.0;
//     }
// }

// // mover via reverse-interpolation
// void mover(double *mesh_value, Points *points) {

//     for (int p = 0; p < NUM_Points; p++) {

//         if (points[p].is_void) continue;

//         double x = points[p].x;
//         double y = points[p].y;

//         int i = (int)(x / dx);
//         int j = (int)(y / dy);

//         if (i >= NX) i = NX - 1;
//         if (j >= NY) j = NY - 1;

//         double Xi = i * dx;
//         double Yj = j * dy;

//         double lx = x - Xi;
//         double ly = y - Yj;

//         // Weights
//         double w00 = (dx - lx) * (dy - ly);
//         double w10 = lx * (dy - ly);
//         double w01 = (dx - lx) * ly;
//         double w11 = lx * ly;

//         int idx = j * GRID_X + i;

//         double Fi =
//             w00 * mesh_value[idx] +
//             w10 * mesh_value[idx + 1] +
//             w01 * mesh_value[idx + GRID_X] +
//             w11 * mesh_value[idx + GRID_X + 1];

//         // Update position
//         points[p].x += Fi * dx;
//         points[p].y += Fi * dy;

//         // Check domain
//         if (points[p].x < 0.0 || points[p].x > 1.0 ||
//             points[p].y < 0.0 || points[p].y > 1.0) {
//             points[p].is_void = 1;
//         }
//     }
// }

// void denormalization(double *mesh_value) {

//     double range = max_val - min_val;

//     if (range == 0.0) return;

//     int size = GRID_X * GRID_Y;

//     for (int i = 0; i < size; i++) {
//         mesh_value[i] = ((mesh_value[i] + 1.0) / 2.0) * range + min_val;
//     }
// }

// // count particles that went beyond the domain
// long long int void_count(Points *points) {

//     long long int voids = 0;
//     for (int i = 0; i < NUM_Points; i++) {
//         voids += (int)points[i].is_void;
//     }
//     return voids;
// }

// // Write mesh to file
// void save_mesh(double *mesh_value) {

//     FILE *fd = fopen("Mesh.out", "w");
//     if (!fd) {
//         printf("Error creating Mesh.out\n");
//         exit(1);
//     }

//     for (int i = 0; i < GRID_Y; i++) {
//         for (int j = 0; j < GRID_X; j++) {
//             fprintf(fd, "%lf ", mesh_value[i * GRID_X + j]);
//         }
//         fprintf(fd, "\n");
//     }

//     fclose(fd);
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "utils.h"

double min_val, max_val;

// ================= INTERPOLATION =================
// Strategy: Thread-private mesh + reduction
void interpolation(double *mesh_value, Points *points) {

    int size = GRID_X * GRID_Y;

    // Reset global mesh
    memset(mesh_value, 0, sizeof(double) * size);

    int nthreads = omp_get_max_threads();

    // Allocate private meshes
    double **local_mesh = (double **)malloc(nthreads * sizeof(double *));
    for (int t = 0; t < nthreads; t++) {
        local_mesh[t] = (double *)calloc(size, sizeof(double));
    }

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        double *mesh_private = local_mesh[tid];

        #pragma omp for schedule(static)
        for (int p = 0; p < NUM_Points; p++) {

            if (points[p].is_void) continue;

            double x = points[p].x;
            double y = points[p].y;
            double f = 1.0;

            int i = (int)(x / dx);
            int j = (int)(y / dy);

            if (i >= NX) i = NX - 1;
            if (j >= NY) j = NY - 1;

            double Xi = i * dx;
            double Yj = j * dy;

            double lx = x - Xi;
            double ly = y - Yj;

            double w00 = (dx - lx) * (dy - ly);
            double w10 = lx * (dy - ly);
            double w01 = (dx - lx) * ly;
            double w11 = lx * ly;

            int idx = j * GRID_X + i;

            mesh_private[idx]                 += w00 * f;
            mesh_private[idx + 1]             += w10 * f;
            mesh_private[idx + GRID_X]        += w01 * f;
            mesh_private[idx + GRID_X + 1]    += w11 * f;
        }
    }

    // Reduction step (combine all thread meshes)
    for (int t = 0; t < nthreads; t++) {
        for (int i = 0; i < size; i++) {
            mesh_value[i] += local_mesh[t][i];
        }
        free(local_mesh[t]);
    }
    free(local_mesh);
}


// ================= NORMALIZATION =================
void normalization(double *mesh_value) {

    int size = GRID_X * GRID_Y;

    min_val = 1e18;
    max_val = -1e18;

    // Parallel min/max
    #pragma omp parallel
    {
        double local_min = 1e18;
        double local_max = -1e18;

        #pragma omp for nowait
        for (int i = 0; i < size; i++) {
            if (mesh_value[i] < local_min) local_min = mesh_value[i];
            if (mesh_value[i] > local_max) local_max = mesh_value[i];
        }

        #pragma omp critical
        {
            if (local_min < min_val) min_val = local_min;
            if (local_max > max_val) max_val = local_max;
        }
    }

    double range = max_val - min_val;
    if (range == 0.0) return;

    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        mesh_value[i] = 2.0 * (mesh_value[i] - min_val) / range - 1.0;
    }
}


// ================= MOVER =================
void mover(double *mesh_value, Points *points) {

    #pragma omp parallel for schedule(static)
    for (int p = 0; p < NUM_Points; p++) {

        if (points[p].is_void) continue;

        double x = points[p].x;
        double y = points[p].y;

        int i = (int)(x / dx);
        int j = (int)(y / dy);

        if (i >= NX) i = NX - 1;
        if (j >= NY) j = NY - 1;

        double Xi = i * dx;
        double Yj = j * dy;

        double lx = x - Xi;
        double ly = y - Yj;

        double w00 = (dx - lx) * (dy - ly);
        double w10 = lx * (dy - ly);
        double w01 = (dx - lx) * ly;
        double w11 = lx * ly;

        int idx = j * GRID_X + i;

        double Fi =
            w00 * mesh_value[idx] +
            w10 * mesh_value[idx + 1] +
            w01 * mesh_value[idx + GRID_X] +
            w11 * mesh_value[idx + GRID_X + 1];

        points[p].x += Fi * dx;
        points[p].y += Fi * dy;

        if (points[p].x < 0.0 || points[p].x > 1.0 ||
            points[p].y < 0.0 || points[p].y > 1.0) {
            points[p].is_void = 1;
        }
    }
}


// ================= DENORMALIZATION =================
void denormalization(double *mesh_value) {

    int size = GRID_X * GRID_Y;

    double range = max_val - min_val;
    if (range == 0.0) return;

    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        mesh_value[i] = ((mesh_value[i] + 1.0) / 2.0) * range + min_val;
    }
}


// ================= VOID COUNT =================
long long int void_count(Points *points) {

    long long int voids = 0;

    #pragma omp parallel for reduction(+:voids)
    for (int i = 0; i < NUM_Points; i++) {
        voids += (int)points[i].is_void;
    }

    return voids;
}


// ================= SAVE =================
void save_mesh(double *mesh_value) {

    FILE *fd = fopen("output_a", "w");
    if (!fd) {
        printf("Error creating output_b\n");
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