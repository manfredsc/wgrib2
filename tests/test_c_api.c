/* This is a test of the wgrib2 c api.
 *
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include "wgrib2_c_test_util.h"
#include <stdio.h>

#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "junk_ftn_api.inv"
#define EXP_GRB_IN "data/gdaswave.t00z.wcoast.0p16.f000.inv"

int
main()
{
    printf("Testing grb2_mk_inv()...\n");
    {
        int ret;

        if ((ret = grb2_mk_inv(GRB_FILE, GRB_INV)))
            return 2;

        if ((ret = compare_grib2_files(GRB_INV, EXP_GRB_IN))) {
            return 3;
        }
    }
    
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}