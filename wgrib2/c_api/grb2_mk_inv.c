/** @file
 * @brief Routine for C API to make an inventory file.
 * @author Public Domain: Wesley Ebisuzaki @date 3/2018
 */
#include <stdio.h>
#include "c_wgrib2api.h"

/* 10/2024  Public Domain  Wesley Ebisuzaki  */
/* C transation of fortran grb2_mk_inv(..) */

int wgrib2(int argc, const char **argv);

/**
 * This function writes the match inventory to a file.
 *
 * @param grb The input GRIB file.
 * @param inv The output inventory file.
 * 
 * @return 0 on success, error code otherwise.
 *
 * @author Wesley Ebisuzaki @date 3/2018
 */
int grb2_mk_inv(char *grb, char *inv) {

    char *argv[7];

    argv[0] = "wgrib2_C_api";
    argv[1] = grb;
    argv[2] = "-rewind_init";
    argv[3] = grb;
    argv[4] = "-inv";
    argv[5] = inv;
    argv[6] = "-Match_inv";
   
    return wgrib2(7, (const char **) argv);
}
