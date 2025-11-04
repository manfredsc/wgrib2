/** @file
 * @brief Implements 3rd NCEP bug fix. NCEP libraries have a bug where they assume 
 * that the dscale is zero when they set the reference values only when nbits == 0.
 * 
 * This fix sets the dscale to zero for NCEP files when nbits == 0 or ngroups == 0.
 * @author Public Domain: Wesley Ebisuzaki @date 02/2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * NCEP libraries have a bug where they assume that the dscale is zero when the
 * set the reference values only when nbits == 0
 *
 * This routine sets the dscale to zero when
 * 1) NCEP file
 * 2) nbits or ngroups == 0
 *
 * the routine fix_ncep_3 is called by wgrib.c before decoding of the file
 *
 * f_fix_ncep_3 is a setup routine, it gets called before the file is read
 *		its location on the command line doesn't matter
 *
 * 2/2009 public domain Wesley Ebisuzaki
 */

/** Flag to indicate if NCEP fix 3 is active.  */
extern int fix_ncep_3_flag;

/*
 * HEADER:200:fix_ncep_3:setup:0:sets flag to fix ncep bug 3 (constant fields)
 */

/**
 * "Set" option that fixes NCEP constant fields.
 *
 * NCEP libraries have a bug where they assume that the dscale is zero when they set 
 * the reference values only when nbits == 0.
 *
 * ## Usage
 * -fix_ncep_3
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success
 *
 * @author Wesley Ebisuzaki @date 02/2009
 */
int f_fix_ncep_3(ARG0) {
    fix_ncep_3_flag = 1;
    return 0;
}

/**
 * Implements NCEP constant fields bug fix. Sets the dscale to zero for NCEP files
 * when nbits == 0 or ngroups == 0. If the file is not NCEP, no changes are made.
 * 
 * @param sec Pointer to the GRIB section.
 *
 * @return 0 for success.
 * 
 * @author Wesley Ebisuzaki @date 02/2009
 */
int fix_ncep_3(unsigned char **sec) {
    int j;

    if (GB2_Center(sec) != NCEP) return 0;		// only for NCEP files

    j = code_table_5_0(sec);
    if (j == 0 || j == 1 || j == 40 || j == 41 || j == 40000 || j == 40010) { // simple, jpeg, png
        if (sec[5][19] == 0) {
            sec[5][17] = sec[5][18] = 0;
        }
    }
    else if (j == 2 || j == 3) {		// complex packing
        if (uint4(sec[5]+31) == 0) {		// number of groups
            sec[5][17] = sec[5][18] = 0;
        }
    }
    else if (j == 50 || j == 51) {		// spectral 
        if (sec[5][19] == 0) {
            sec[5][17] = sec[5][18] = 0;
        }
    }
    return 0;
}
