/* This is a test of the wgrib2 c api.
 *
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include <stdio.h>

#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "data/gdaswave.t00z.wcoast.0p16.f000.inv"

int
main()
{
    printf("Testing grb2_mk_inv()...\n");
    {
        int ret;

        if ((ret = grb2_mk_inv(GRB_FILE, GRB_INV)))
            return ret;

    }
    
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}