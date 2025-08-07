/** @file
 * @brief Check the size of the Product Definition Template (PDT, Section 4).
 * @author Public Domain: Wesley Ebisuzaki @date 09/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* 
 * Check_pdt_size.c     10/2024 Public Domain Wesley Ebisuzaki
 *
 * check_pdt_size(..) checks the size of the pdt
 *   sees whether the actual size of the pdt is the expected value
 *
 * This check can be enabled or disabled by -check_pdt 1/0  (enable/disable)
 *
 * in theory, the pdt can be bigger than expected with no ill consequences
 *  howerver, if the pdt is smaller than expectations, then any routine that
 *  uses the pdt could be reading outside of the pdt.
 */

/*
 * HEADER:100:check_pdt_size:misc:1:check pdt size X=1 enable/default, X=0 disable
 */

 /** Flag to indicate whether to check PDT size. */
int check_pdt_size_flag = 1;

/** Flag to indicate whether to issue warnings for PDT size checks. */
int warn_check_pdt = 1;

/**
 * Function to set the check_pdt_size_flag.
 * 
 * If the PDT is wrong size, wgrib2 will exit with a delayed fatal error. 
 * The fatal error is delayed so the user can investigate the contents of the 
 * bad grib message. Note that it is possible to seg fault because Section 4 is a 
 * different size than expected, and the rest of the grib message may be corrupted. 
 * 
 * You may skip this check by using the -check_pdt_size 0 option.
 * 
 * @param ARG1 ???
 * 
 * @return 0 for success.
 * 
 * @note In theory, the pdt can be bigger than expected with no ill consequences,
 *  however, if the pdt is smaller than expectations, then any routine that
 *  uses the pdt could be reading outside of the pdt.
 * 
 * ## Usage:
 * - check_pdt_size     0 (disable)
 *                      1 (enable, default)
 * 
 * @author Wesley Ebisuzaki @date 09/2020
 */
int f_check_pdt_size(ARG1) {
    check_pdt_size_flag = atoi(arg1);
    return 0;
}

/**
 * Checks the size of the Product Definition Template (PDT) in the GRIB message.
 * 
 * @param sec Pointer to the GRIB message sections.
 * 
 * @return 0 for success, 1 if the PDT size is incorrect.
 * 
 * @author Wesley Ebisuzaki @date 09/2020
 */
int check_pdt_size(unsigned char **sec) {
    int calc_pdt_size, pdt_size;

    if (check_pdt_size_flag == 0) return 1;

    calc_pdt_size = pdt_len(sec, -1);
    pdt_size =  GB2_Sec4_size(sec);
    // fprintf(stderr, "pdt_size: %d %d\n",  pdt_size, calc_pdt_size);

    if (calc_pdt_size == -1) {
        fprintf(stderr,"check_pdt_size: pdt=%d needs to be added to pdt_len(..)\n", code_table_4_0(sec));
        return 1;
    }
    if (pdt_size == calc_pdt_size) return 1;
    if (warn_check_pdt++ < 4) fprintf(stderr,"*** check_pdt: pdt size %d expected %d ***\n",
        pdt_size, calc_pdt_size);
    return 0;
}
