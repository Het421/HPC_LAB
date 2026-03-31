#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "utils.h"

// ---------------------------------
// INTERPOLATION (FIXED)
// ---------------------------------
void interpolation(double * __restrict__ mesh_value,
                   Points * __restrict__ points)
{

    for (long long p = 0; p < NUM_Points; p++)
    {
        double px = points[p].x;
        double py = points[p].y;

        int i = (int)(px / dx);
        int j = (int)(py / dy);

        if (i >= NX - 1) i = NX - 2;
        if (j >= NY - 1) j = NY - 2;

        double x_i = i * dx;
        double y_j = j * dy;

        double hx = (px - x_i) / dx;
        double hy = (py - y_j) / dy;

        mesh_value[j * GRID_X + i] += (1.0 - hx) * (1.0 - hy);
        mesh_value[j * GRID_X + (i + 1)] += hx * (1.0 - hy);
        mesh_value[(j + 1) * GRID_X + i] += (1.0 - hx) * hy;
        mesh_value[(j + 1) * GRID_X + (i + 1)] += hx * hy;
    }
}

// ---------------------------------
// SERIAL IMMEDIATE MOVER
// ---------------------------------
void mover_serial_immediate(Points * __restrict__ points,
                            double deltaX, double deltaY)
{
    for (long long p = 0; p < NUM_Points; p++)
    {
        double r_x = ((double)rand() / RAND_MAX) * 2.0 * deltaX - deltaX;
        double r_y = ((double)rand() / RAND_MAX) * 2.0 * deltaY - deltaY;

        double new_x = points[p].x + r_x;
        double new_y = points[p].y + r_y;

        if (new_x < 0.0 || new_x >= 1.0 || new_y < 0.0 || new_y >= 1.0)
        {
            points[p].x = (double)rand() / RAND_MAX;
            points[p].y = (double)rand() / RAND_MAX;
        }
        else
        {
            points[p].x = new_x;
            points[p].y = new_y;
        }
    }
}

// ---------------------------------
// SERIAL DEFERRED MOVER (USED)
// ---------------------------------
void mover_serial_deferred(Points * __restrict__ points,
                           double deltaX, double deltaY)
{
    // Move particles
    for (long long p = 0; p < NUM_Points; p++)
    {
        double r_x = ((double)rand() / RAND_MAX) * 2.0 * deltaX - deltaX;
        double r_y = ((double)rand() / RAND_MAX) * 2.0 * deltaY - deltaY;

        points[p].x += r_x;
        points[p].y += r_y;
    }

    long long left = 0;
    long long right = NUM_Points - 1;

    // Partition valid particles
    while (left <= right)
    {
        int left_valid =
            (points[left].x >= 0.0 && points[left].x < 1.0 &&
             points[left].y >= 0.0 && points[left].y < 1.0);

        if (left_valid)
        {
            left++;
        }
        else
        {
            int right_valid =
                (points[right].x >= 0.0 && points[right].x < 1.0 &&
                 points[right].y >= 0.0 && points[right].y < 1.0);

            if (!right_valid)
            {
                right--;
            }
            else
            {
                Points temp = points[left];
                points[left] = points[right];
                points[right] = temp;
                left++;
                right--;
            }
        }
    }

    // Reinitialize invalid particles
    for (long long p = left; p < NUM_Points; p++)
    {
        points[p].x = (double)rand() / RAND_MAX;
        points[p].y = (double)rand() / RAND_MAX;
    }
}

// ---------------------------------
// PARALLEL IMMEDIATE MOVER
// ---------------------------------
void mover_parallel_immediate(Points * __restrict__ points,
                              double deltaX, double deltaY)
{
#pragma omp parallel
    {
        unsigned int seed = 12345 + omp_get_thread_num();

#pragma omp for schedule(static)
        for (long long p = 0; p < NUM_Points; p++)
        {
            double r_x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 * deltaX - deltaX;
            double r_y = ((double)rand_r(&seed) / RAND_MAX) * 2.0 * deltaY - deltaY;

            double new_x = points[p].x + r_x;
            double new_y = points[p].y + r_y;

            if (new_x < 0.0 || new_x >= 1.0 || new_y < 0.0 || new_y >= 1.0)
            {
                points[p].x = (double)rand_r(&seed) / RAND_MAX;
                points[p].y = (double)rand_r(&seed) / RAND_MAX;
            }
            else
            {
                points[p].x = new_x;
                points[p].y = new_y;
            }
        }
    }
}

// ---------------------------------
// PARALLEL DEFERRED MOVER
// ---------------------------------
void mover_parallel_deferred(Points * __restrict__ points,
                             double deltaX, double deltaY)
{
#pragma omp parallel
    {
        unsigned int seed = 12345 + omp_get_thread_num();

#pragma omp for schedule(static)
        for (long long p = 0; p < NUM_Points; p++)
        {
            double r_x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 * deltaX - deltaX;
            double r_y = ((double)rand_r(&seed) / RAND_MAX) * 2.0 * deltaY - deltaY;

            points[p].x += r_x;
            points[p].y += r_y;
        }
    }

    long long left = 0;
    long long right = NUM_Points - 1;

    while (left <= right)
    {
        int left_valid =
            (points[left].x >= 0.0 && points[left].x < 1.0 &&
             points[left].y >= 0.0 && points[left].y < 1.0);

        if (left_valid)
        {
            left++;
        }
        else
        {
            int right_valid =
                (points[right].x >= 0.0 && points[right].x < 1.0 &&
                 points[right].y >= 0.0 && points[right].y < 1.0);

            if (!right_valid)
            {
                right--;
            }
            else
            {
                Points temp = points[left];
                points[left] = points[right];
                points[right] = temp;
                left++;
                right--;
            }
        }
    }

#pragma omp parallel
    {
        unsigned int seed = 12345 + omp_get_thread_num();

#pragma omp for schedule(static)
        for (long long p = left; p < NUM_Points; p++)
        {
            points[p].x = (double)rand_r(&seed) / RAND_MAX;
            points[p].y = (double)rand_r(&seed) / RAND_MAX;
        }
    }
}

// ---------------------------------
// SAVE MESH
// ---------------------------------
void save_mesh(double *mesh_value)
{
    FILE *fd = fopen("Mesh.out", "w");

    if (!fd)
    {
        printf("Error creating Mesh.out\n");
        exit(1);
    }

    for (int i = 0; i < GRID_Y; i++)
    {
        for (int j = 0; j < GRID_X; j++)
        {
            fprintf(fd, "%lf ", mesh_value[i * GRID_X + j]);
        }
        fprintf(fd, "\n");
    }

    fclose(fd);
}