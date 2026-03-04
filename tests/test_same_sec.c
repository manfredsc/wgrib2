/**
 * This is a test for the wgrib2 project. This test is for all 
 * of the same_sec*() routines in test_sec.c.
 * 
 * Alyson Stahl, 3/2026
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"

int
main()
{
    printf("Testing same_sec*() routines.\n");
    printf("Testing sec0 routines...\n");
    {
        unsigned char sec0_a[16] = {
            0x47, 0x52, 0x49, 0x42,     /* "GRIB" */
            0, 0,                       /* Reserved */
            0,                          /* Discipline */
            2,                          /* Edition Number */
            0, 0, 0, 0, 0, 0, 0, 0      /* Total Length (not compared) */
        };
        unsigned char sec0_b[16] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 16; i++) sec0_b[i] = sec0_a[i];

        sec_a[0] = sec0_a;
        sec_b[0] = sec0_b;

        /* identical sections, should return 1 */
        if (!same_sec0(sec_a, sec_b)) {
            printf("same_sec0: identical sections should return 1\n");
            return 10;
        }

        /* different edition number, should return 0 */
        sec0_b[7] = 1;
        if (same_sec0(sec_a, sec_b)) {
            printf("same_sec0: different edition number should return 0\n");
            return 11;
        }

        /* identical sections, but different edition number, should return 1 */
        if (!same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: should ignore edition number and return 1\n");
            return 12;
        }

        /* different discipline, should return 0 */
        sec0_b[6] = 1;
        sec0_b[7] = 2; /* reset edition number to original value */
        if (same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: different discipline should return 0\n");
            return 13;
        }
    }

    printf("SUCCESS!\n");
    return 0;
}