/* This is a test of the wgrib2 c api.
 *
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include "wgrib2_test_util.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "junk_c_api.inv"
#define EXP_GRB_INV "data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv"

int
main()
{
    printf("Testing grb2_mk_inv()...\n");
    {
        int ret;

        if ((ret = grb2_mk_inv(GRB_FILE, GRB_INV))) {
            printf("grb2_mk_inv() failed with return code %d\n", ret);
            return 2;
        }

        if ((ret = compare_files(GRB_INV, EXP_GRB_INV))) {
            printf("Inventory files differ.\n");
            return 3;
        }
    }
    printf("ok!\n");
    printf("Testing wgrib2_add_cmd()...\n");
    printf("Testing with overly long command string...\n");
    fflush(stdout);
    {
        char longopt[CMD_LEN + 1];

        wgrib2_init_cmds();
        memset(longopt, 'A', CMD_LEN);
        longopt[CMD_LEN] = '\0';
        wgrib2_add_cmd(longopt);
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}
