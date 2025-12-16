/**
 * This is the header file for wgrib2_test_util.c.
 * 
 * Alyson Stahl 9/18/2025
 */

#define MAX_FILENAME_LEN 256

int compare_files(char *fname1, char *fname2);
int compare_grib2_files(char *fname1, char *fname2);