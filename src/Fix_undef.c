/** @file
 * @brief Sets unused fields to undefined values.
 * 
 * Some sections have unused fields - the contents of these fields 
 * can be anything. This causes problems with section to section comparisons.
 * @author Wesley Ebisuzaki @date 02/2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Flag to indicate if undefined value fixing is active. */
extern int fix_undef_flag;

/*
 * HEADER:100:fix_undef:setup:0:set unused values to undef
 */

/**
 * "Set" option that activates undefined value fixing. 
 * 
 * Some sections have unused fields - the contents of these fields 
 * can be anything. This causes problems with section to section comparisons.
 * 
 * ## Usage
 * -fix_undef
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 * 
 * @author Wesley Ebisuzaki @date 02/2020
 */
int f_fix_undef(ARG0) {
    fix_undef_flag = 1;
    return 0;
}

/**
 * Fix unused fields in GRIB2 sections by setting them to undefined values.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return 0 for success
 * 
 * @author Wesley Ebisuzaki @date 02/2020
 */
int fix_undef(unsigned char **sec) {

    int center, gdt;
    unsigned char *p;
 
    gdt = code_table_3_1(sec);
    center = GB2_Center(sec);
    p = NULL;

    /* get basic angle for grids with subangle */
    switch (gdt) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 40:
        case 41:
        case 42:
        case 43:
            p = &(sec[3][38]);
            break;
        case 32768:
        case 32769:
            if (center == NCEP) p = &(sec[3][38]);
            break;
    }

    if (p) {
        /* if basic angle is zero, subangle is undefined */
        if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0) {
            p[4] = p[5] = p[6] = p[7] = 255;
        }
    }
    return 0;
}
