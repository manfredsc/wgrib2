/*
 * These are utility functions for wgrib2 tests.
 * 
 * Based on some functions in g2c_test_util.c.
 * 
 * Alyson Stahl 9/18/2025
 */

#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024

/*
 * Compare two strings, ignoring leading spaces.
 * 
 * Ed Hartnett 10/6/22
 */
static int cmpString(const void *p, const void *q) {
    const char *pp = (const char *)p;
    const char *qq = (const char *)q;
    int i, j;
    for (i = 0; isblank(pp[i]); ++i);
    for (j = 0; isblank(qq[j]); ++j);
    return strcmp(pp + i, qq + j);
}

/**
 * Compare two files line by line, ignoring leading spaces.
 * 
 * Returns 0 if the files are the same.
 * 
 * Alyson Stahl 9/18/2025
 */
int compare_files(char *fname1, char *fname2) {
    
    FILE *fp1, *fp2;
    char line1[MAX_LINE_LEN + 1], line2[MAX_LINE_LEN + 1];
    int line_num = 0;
    int result = 0;

    if (!(fp1 = fopen(fname1, "r"))) {
        fprintf(stderr, "Could not open file %s\n", fname1);
        return 1;
    }
    if (!(fp2 = fopen(fname2, "r"))) {
        fprintf(stderr, "Could not open file %s\n", fname1);
        return 1;
    }

    while (fgets(line1, MAX_LINE_LEN, fp1)) {
        if (!fgets(line2, MAX_LINE_LEN, fp2)) {
            result = 1;
            break;
        }

        if (cmpString(line1, line2) != 0) {
            result = 1;
            break;
        }
    }

    fclose(fp1);
    fclose(fp2);

    return result;
}