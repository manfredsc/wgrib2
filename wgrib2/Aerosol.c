/** @file
 * @brief Functions for handling aerosol size and wavelength in GRIB files.
 * @author Public Domain: Wesley Ebisuzaki @date 02/2012
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 02/2012 | W. Ebisuzaki | Initial
 * 10/2024 | W. Ebisuzaki | Fix to handle all pdts
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:200:aerosol_size:inv:0:optical properties of an aerosol
 */

/**
 * Prints the size of the aerosol particle if applicable. This product applies to Product Definition 
 * Template 4.44-4.48. This option is part of the standard inventory.
 * 
 * ## Usage
 * -aerosol_size
 * 
 * @param ARG0 ???
 *
 * @return 0 for success, does not return error codes.
 * 
 * ## Example: 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 02/2012
 */
int f_aerosol_size(ARG0) {
    unsigned char *ptr;
    double size1, size2;

    if (mode >= 0) {
        ptr = code_table_4_91_location(sec);
        if (ptr == NULL) return 0;
        if (*ptr == 255) return 0;
        size1 = scaled2dbl(INT1(ptr[1]), int4(ptr+2));
        size2 = scaled2dbl(INT1(ptr[6]), int4(ptr+7));

        sprintf(inv_out,"aerosol_size ");
        inv_out += strlen(inv_out);
        prt_code_table_4_91(ptr[0], size1, size2, inv_out);
    }
    return 0;
}
/*
 * HEADER:200:aerosol_wavelength:inv:0:optical properties of an aerosol
 */

/**
 * Prints the optical properties of the aerosol particle. This option is part of the standard inventory.
 * 
 * 
 * ## Usage
 * -aerosol_wavelength
 * 
 * @param ARG0 ???
 *
 * @return 0 for success, does not return error codes.
 * 
 * ## Example: 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 02/2012
 */
int f_aerosol_wavelength(ARG0) {
    unsigned char *ptr;
    double wave1, wave2;

    if (mode >= 0) {
        ptr = code_table_4_91b_location(sec);
        if (ptr == NULL) return 0;
        if (*ptr == 255) return 0;
        wave1 = scaled2dbl(INT1(ptr[1]), int4(ptr+2));
        wave2 = scaled2dbl(INT1(ptr[6]), int4(ptr+7));

        sprintf(inv_out,"aerosol_wavelength ");
        inv_out += strlen(inv_out);
        prt_code_table_4_91(ptr[0], wave1, wave2, inv_out);

    }
    return 0;
}

