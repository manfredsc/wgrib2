/**
 * This is a test for the wgrib2 project. This test is for units.c.
 * 
 * Alyson Stahl, 2/2026
 */
#include <stdio.h>
#include "wgrib2.h"

extern int ftime_mode;

#define NUM_TR_ENTRIES 13

static const int tr_ints[NUM_TR_ENTRIES] = {
    0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 255
};

static const char *tr_names[NUM_TR_ENTRIES] = {
    "min",
    "hour",
    "day",
    "month",
    "year",
    "decade",
    "normal",
    "century",
    "3-hours",
    "6-hours",
    "12-hours",
    "sec",
    "missing",
};

int
main()
{
    printf("Testing a2time_range().\n");
    printf("Testing a2time_range() with every valid time range name...\n");
    {
        for (int i = 0; i < NUM_TR_ENTRIES; i++) {
            int actual = a2time_range(tr_names[i]);
            int expected = tr_ints[i];
            if (actual != expected) {
                printf("Error: a2time_range() returned %d for input %s, expected %d.\n",
                        actual, tr_names[i], expected);
                return 10;
            }
        }
    }
    printf("Testing a2time_range() with an invalid name. Should return -1...\n");
    {
        int result = a2time_range("invalid");
        if (result != -1) {
            printf("Error: a2time_range() returned %d for invalid input, expected -1.\n",
                    result);
            return 11;
        }
    }
    printf("Testing time_range2a().\n");
    printf("Testing time_range2a() with every valid time range integer...\n");
    {
        for (int i = 0; i < NUM_TR_ENTRIES; i++) {
            const char *actual = time_range2a(tr_ints[i]);
            const char *expected = tr_names[i];

            int j = 0;
            while (actual[j] != '\0' && expected[j] != '\0' && actual[j] == expected[j]) {
                j++;
            }
            if (actual[j] != expected[j]) {
                printf("Error: time_range2a() returned %s for input %d, expected %s.\n",
                        actual, tr_ints[i], expected);
                return 12;
            }
        }
    }
    printf("Testing time_range2a() with an invalid integer. Should return \"?\"...\n");
    {
        const char *result = time_range2a(-1);

        const char *expected = "?";
        int j = 0;
        while (result[j] != '\0' && expected[j] != '\0' && result[j] == expected[j]) {
            j++;
        }
        if (result[j] != expected[j]) {
            printf("Error: time_range2a() returned %s for invalid input, expected %s.\n",
                    result, expected);
            return 13;
        }
    }
    printf("Testing normalize_time_range().\n");
    printf("Testing normalize_time_range() with every time range. Unimplemented cases should do nothing...\n");
    {

        int base_val[NUM_TR_ENTRIES] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
        int exp_tr[NUM_TR_ENTRIES]  = { 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 1, 13, 255 };
        int exp_val[NUM_TR_ENTRIES] = { 2, 2, 2, 2, 2, 2, 2, 2, 6, 12, 24, 2, 2 };

        for (int i = 0; i < NUM_TR_ENTRIES; i++) {
            int tr = tr_ints[i];
            int val = base_val[i];

            normalize_time_range(&tr, &val);

            if (tr != exp_tr[i] || val != exp_val[i]) {
                printf("Error: normalize_time_range() gave tr=%d, val=%d for input tr=%d, val=%d; expected tr=%d, val=%d.\n",
                        tr, val, tr_ints[i], base_val[i], exp_tr[i], exp_val[i]);
                return 14;
            }
        }
    }
    printf("Testing simple_time_range().\n");
    printf("Testing simple_time_range() with every time range and ftime_mode = 0. Unimplemented cases should do nothing...\n");
    {
        ftime_mode = 0;

        int base_val[NUM_TR_ENTRIES] = { 120, 48, 2, 2, 2, 2, 2, 2, 2, 2, 2, 180, 2 };
        int exp_tr[NUM_TR_ENTRIES]  = { 1, 2, 2, 3, 4, 5, 6, 7, 1, 1, 2, 0, 255 };
        int exp_val[NUM_TR_ENTRIES] = { 2, 2, 2, 2, 2, 2, 2, 2, 6, 12, 1, 3, 2 };

        for (int i = 0; i < NUM_TR_ENTRIES; i++) {
            int tr = tr_ints[i];
            int val = base_val[i];

            simple_time_range(&tr, &val);

            if (tr != exp_tr[i] || val != exp_val[i]) {
                printf("Error: simple_time_range(ftime_mode=0) gave tr=%d, val=%d for input tr=%d, val=%d; expected tr=%d, val=%d.\n",
                        tr, val, tr_ints[i], base_val[i], exp_tr[i], exp_val[i]);
                return 15;
            }
        }
    }
    printf("Testing simple_time_range() with every time range and ftime_mode = 1. Unimplemented cases should do nothing...\n");
    {
        ftime_mode = 1;

        int base_val[NUM_TR_ENTRIES] = { 120, 48, 2, 2, 2, 2, 2, 2, 2, 2, 2, 180, 2 };
        int exp_tr[NUM_TR_ENTRIES]  = { 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 1, 13, 255 };
        int exp_val[NUM_TR_ENTRIES] = { 120, 48, 2, 2, 2, 2, 2, 2, 6, 12, 24, 180, 2 };

        for (int i = 0; i < NUM_TR_ENTRIES; i++) {
            int tr = tr_ints[i];
            int val = base_val[i];

            simple_time_range(&tr, &val);

            if (tr != exp_tr[i] || val != exp_val[i]) {
                printf("Error: simple_time_range(ftime_mode=1) gave tr=%d, val=%d for input tr=%d, val=%d; expected tr=%d, val=%d.\n",
                        tr, val, tr_ints[i], base_val[i], exp_tr[i], exp_val[i]);
                return 16;
            }
        }

        /** Set back to default. */
        ftime_mode = 0;
    }
    printf("Testing a2code_4_10() and code_4_10_name().\n");
    printf("Testing a2code_4_10() with valid inputs and checking against code_4_10_name() results...\n");
    {
        for (int i = 0; i <= 8; i++) {
            const char *name = code_4_10_name(i, 0);
            int code = a2code_4_10(name);
            if (code != i) {
                printf("Error: a2code_4_10() returned %d for input %s, expected %d.\n",
                        code, name, i);
                return 17;
            }
        }
    }
    printf("Testing a2code_4_10() with invalid input. Should return -1...\n");
    {
        if (a2code_4_10("invalid") != -1) {
            printf("Error: a2code_4_10() did not return -1 for invalid input.\n");
            return 18;
        }
    }
    printf("Testing code_4_10_name() with invalid input. Should return \"???\"...\n");
    {
        const char *invalid_name = code_4_10_name(-1, 0);
        const char *expected = "???";
        int j = 0;
        while (invalid_name[j] != '\0' && expected[j] != '\0' && invalid_name[j] == expected[j]) {
            j++;
        }
        if (invalid_name[j] != expected[j]) {
            printf("Error: code_4_10_name() did not return \"???\" for invalid input.\n");
            return 19;
        }
    }
    printf("Testing a2anl_fcst().\n");
    {
        if (a2anl_fcst("anl") != 0) {
            printf("Error: a2anl_fcst() did not return 0 for input \"anl\".\n");
            return 20;
        }

        if (a2anl_fcst("fcst") != 1) {
            printf("Error: a2anl_fcst() did not return 1 for input \"fcst\".\n");
            return 21;
        }

        if (a2anl_fcst("invalid") != -1) {
            printf("Error: a2anl_fcst() did not return -1 for invalid input.\n");
            return 22;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}