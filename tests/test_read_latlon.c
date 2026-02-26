/**
 * This is a test for the wgrib2 project. This test is for read_latlon.c.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include <setjmp.h>
#include "wgrib2.h"

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing read_latlon().\n");
    printf("Testing standard call of read_latlon()...\n");
    {
        const char *arg = "-10:0:370:10:100:20";
        double *lon = NULL, *lat = NULL;
        unsigned int n;
        double expected_lon[3] = {350.0, 10.0, 100.0};
        double expected_lat[3] = {0.0, 10.0, 20.0};

        n = read_latlon(arg, &lon, &lat);
        if (n != 3) {
            printf("Expected n_out = %u, got %u.\n", 3u, n);
            return 10;
        }

        for (unsigned int i = 0; i < n; i++) {
            if (lon[i] != expected_lon[i] || lat[i] != expected_lat[i]) {
                printf("Index %u: expected lon = %.6f, lat = %.6f; got lon = %.6f, lat = %.6f.\n",
                       i, expected_lon[i], expected_lat[i], lon[i], lat[i]);
                return 11;
            }
        }
    }
    printf("Testing read_latlon() with no grid point locations. Should throw a fatal error....\n");
    {
        double *lon = NULL, *lat = NULL;

        if (setjmp(fatal_err) == 0) {
            (void) read_latlon("x:y", &lon, &lat);
            return 12;
        }
    }
    printf("Testing read_latlon() with missing grid point locations. Should throw a fatal error....\n");
    {
        double *lon = NULL, *lat = NULL;

        if (setjmp(fatal_err) == 0) {
            (void) read_latlon("10:20:30:bad", &lon, &lat);
            return 13;
        }
    }
    printf("Testing read_latlon() with incomplete argument string. Should return 0...\n");
    {
        double *lon = NULL, *lat = NULL;
        unsigned int n;

        n = read_latlon("10:20:30", &lon, &lat);
        if (n != 0) {
            return 14;
        }
    }
    printf("Testing read_latlon() with latitude < -90. Should return 0...\n");
    {
        double *lon = NULL, *lat = NULL;
        unsigned int n;

        n = read_latlon("10:-91", &lon, &lat);
        if (n != 0) {
            return 15;
        }
    }
    printf("Testing read_latlon() with latitude > 90. Should return 0...\n");
    {
        double *lon = NULL, *lat = NULL;
        unsigned int n;

        n = read_latlon("10:91", &lon, &lat);
        if (n != 0) {
            return 16;
        }
    }
    printf("Testing read_latlon() with adjusted longitude < 0. Should return 0...\n");
    {
        double *lon = NULL, *lat = NULL;
        unsigned int n;

        n = read_latlon("-400:0", &lon, &lat);
        if (n != 0) {
            return 17;
        }
    }
    printf("Testing read_latlon() with adjusted longitude > 360. Should return 0...\n");
    {
        double *lon = NULL, *lat = NULL;
        unsigned int n;

        n = read_latlon("800:0", &lon, &lat);
        if (n != 0) {
            return 18;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}