/**
 * This is a test for the wgrib2 project. This test is for string2time_unit.c.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include "wgrib2.h"

#define NUM_TESTS 6

int
main()
{
    printf("Testing string2time_unit().\n");
    {
        char *test_strings[NUM_TESTS] = {"hr", "dy", "mo", "yr", "mn", "invalid"};
        int expected_results[NUM_TESTS] = {1, 2, 3, 4, 0, -1};
        for (int i = 0; i < NUM_TESTS; i++) {
            int result = string2time_unit(test_strings[i]);
            if (result != expected_results[i]) {
                printf("Test failed for input '%s': expected %d, got %d\n",
                       test_strings[i], expected_results[i], result);
                return 10;
            }
        }
    }
    printf("SUCCESS!\n");
    return 0;
}