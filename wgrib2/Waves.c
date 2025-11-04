/** @file
 * @brief Inventory for Ocean Surface Wave Partition (PDT=4.52)
 * @author Public Domain: Wesley Ebisuzaki @date 2/2012
 */
#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:200:wave_partition:inv:0:ocean surface wave partition (pdt=4.52)
 */

/**
 * Prints out inventory information for the ocean surface wave partition (PDT 4.52).
 * 
 * ## Usage
 * -wave_partition
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 2/2012
 */
int f_wave_partition(ARG0) {
    int pdt;
    if (mode >= 0) {
        pdt = GB2_ProdDefTemplateNo(sec);
        if (pdt == 52) {
            if (sec[4][13] == 255) sprintf(inv_out,"wave partition=?");
            else sprintf(inv_out,"wave partition=%d",(int) sec[4][13]);
            inv_out += strlen(inv_out);
            if (sec[4][12] == 255) sprintf(inv_out,"/?");
            else sprintf(inv_out,"/%d",(int) sec[4][12]);
        }
    }
    return 0;
}
