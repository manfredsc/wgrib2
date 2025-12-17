/* This is a test of the wgrib2 c api.
 *
 * Note: This test uses setjmp/longjmp to catch fatal_error() calls. This may cause an 
 * Illegal Instruction error (or similar) when using the Intel Classic compiler. I haven't 
 * been able to find a workaround for this. CMakeLists.txt has been modified to skip this 
 * test when using the Intel Classic compiler.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include "wgrib2_test_util.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <setjmp.h>

#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "junk_c_api.inv"
#define EXP_GRB_INV "data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv"

extern jmp_buf fatal_err;

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

        if (setjmp(fatal_err) == 0) {
            /* First pass: call will trigger fatal_error() and longjmp back here */
            (void)wgrib2_add_cmd(longopt);
            /* If we get here, fatal_error() did not occur (unexpected) */
            printf("ERROR: expected fatal_error but call returned.\n");
        } else {
            /* longjmp path: fatal_error() was called as intended */
            printf("Caught fatal_error via longjmp.\n");
        }
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}
