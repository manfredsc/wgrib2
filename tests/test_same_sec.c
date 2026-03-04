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
    printf("Testing same_sec0()...\n");
    {
        unsigned char sec0_a[16] = {
            'G', 'R', 'I', 'B', 
            0, 0,                   /* reserved */
            0, 2,                   /* discipline = 0, edition = 2 */
            0, 0, 0, 0, 0, 0, 0, 0  /* total length (not checked) */
        };
        unsigned char sec0_b[16] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        /**
         * same_sec0() does not compare beyond byte 8 (total length). 
         * Test will break if same_sec0() ever compares all 16 bytes.
         */
        for (int i = 0; i < 8; i++) sec0_b[i] = sec0_a[i];
        for (int i = 8; i < 16; i++) sec0_b[i] = 255;
        
        sec_a[0] = sec0_a;
        sec_b[0] = sec0_b;

        if (!same_sec0(sec_a, sec_b)) {
            printf("same_sec0: sections should be the same\n");
            return 1;
        }

        for (int i = 0; i < 8; i++) {
            sec0_b[i] = 255;
            if (same_sec0(sec_a, sec_b)) {
                printf("same_sec0: sections should be different at byte %d\n", i);
                return 1;
            }
            sec0_b[i] = sec0_a[i];
        }
    }
    printf("Testing same_sec0_not_var()...\n");
    {
        unsigned char sec0_a[16] = {
            'G', 'R', 'I', 'B', 
            0, 0,                   /* reserved */
            0, 2,                   /* discipline = 0, edition = 2 */
            0, 0, 0, 0, 0, 0, 0, 0  /* total length (not checked) */
        };
        unsigned char sec0_b[16] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        /**
         * same_sec0() does not compare beyond byte 8 (total length). 
         * Test will break if same_sec0() ever compares all 16 bytes.
         */
        for (int i = 0; i < 8; i++) sec0_b[i] = sec0_a[i];
        for (int i = 8; i < 16; i++) sec0_b[i] = 255;
        
        sec_a[0] = sec0_a;
        sec_b[0] = sec0_b;

        if (!same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: sections should be the same\n");
            return 1;
        }

        /* different edition, should be skipped */
        sec0_b[7] = 1;
        if (!same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: different edition should return 1\n");
            return 1;
        }
        sec0_b[7] = sec0_a[7];

        for (int i = 0; i < 8; i++) {
            if (i == 7) continue; /* ignores GRIB variable */
            sec0_b[i] = 255;
            if (same_sec0_not_var(1, sec_a, sec_b)) {
                printf("same_sec0_not_var: sections should be different at byte %d\n", i);
                return 1;
            }
            sec0_b[i] = sec0_a[i];
        }

    }

    printf("SUCCESS!\n");
    return 0;
}