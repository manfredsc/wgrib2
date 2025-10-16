/** @file
 * @brief Show hybrid coordinate metadata.
 * @author Public Domain: Wesley Ebisuzaki @date 7/2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:400:hybrid:inv:0:shows vertical coordinate parameters from Sec4 (assuming 2 var per level
 */

/**
 * Prints vertical coordinate parameters from Section 4 (assuming 2 var per level).
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 7/2014
 */
int f_hybrid(ARG0) {
    int calc_pdtsize, pdtsize, n, k;
    if (mode >= 0) {
        n = uint2(&(sec[4][5]));                // number of vert coordinate values
        if (n == 0) return 0;

        calc_pdtsize = pdt_len(sec, -1);	// calculated size of PDT
        pdtsize = GB2_Sec4_size(sec);		// size of PDT + vert coordinatese + extras

        if (calc_pdtsize <= 0) return 0;		// no calculated pdt size
        if (calc_pdtsize > pdtsize) 
            fatal_error_i("not enough space allocated for vertical coordinate data, vert_cordinate values=%d", n);
        if (calc_pdtsize != pdtsize) return 0;

        sprintf(inv_out,"vertical coordinate parameters ");
        inv_out += strlen(inv_out);
        if (n % 2 != 0) {
            for (k = 0; k < n; k++) {
                sprintf(inv_out," %d=%9.6f", k+1, 
            (double) ieee2flt(sec[4]+pdtsize-n*4+k));
                inv_out += strlen(inv_out);
            }
            return 0;
        }
        else {
            for (k = 0; k < n/2; k++) {
            sprintf(inv_out," %d=(%9.6f,%9.6f)", k+1, 
                (double) ieee2flt(sec[4]+pdtsize-n*4+k*2),
                (double) ieee2flt(sec[4]+pdtsize-n*4+k*2+4));
                inv_out += strlen(inv_out);
            }
        }
    }
    return 0;
}
