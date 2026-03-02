/**
 * This is a test for the wgrib2 project. This test is for itoshort_a.c.
 * 
 * Alyson Stahl, 3/2026
 */

#include <stdio.h>
#include "wgrib2.h"

int
main()
{
    printf("Testing itoshort_a().\n");
    printf("Testing with 0. Should return '0'...\n");
    {
        char buf[20];
        itoshort_a(buf, 0);
        if (strcmp(buf, "0") != 0) {
            return 10;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}