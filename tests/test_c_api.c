/* This is a test of the wgrib2 c api.
 *
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include "wgrib2_c_test_util.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "junk_ftn_api.inv"
#define EXP_GRB_IN "data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv"

static volatile int fatal_error_called = 0;

/** Override fatal_error for testing */
void fatal_error(const char *fmt, const char *arg) {
    (void)fmt; (void)arg;
    fatal_error_called = 1;
}

int
main()
{
    /**printf("Testing grb2_mk_inv()...\n");
    fflush(stdout);
    {
        int ret;

        if ((ret = grb2_mk_inv(GRB_FILE, GRB_INV))) {
            printf("grb2_mk_inv() failed with return code %d\n", ret);
            fflush(stdout);
            return 2;
        }
        

        if ((ret = compare_files(GRB_INV, EXP_GRB_IN))) {
            printf("Inventory files differ.\n");
            fflush(stdout);
            return 3;
        }
    }
    printf("ok!\n");
    */
    printf("Testing wgrib2_add_cmd()...\n");
    fflush(stdout);
    {
        printf("Testing overly long command string...\n");
        fflush(stdout);
        
        char longopt[CMD_LEN + 1];
        fatal_error_called = 0;
        wgrib2_init_cmds();

        memset(longopt, 'A', CMD_LEN);
        longopt[CMD_LEN] = '\0';
        wgrib2_add_cmd(longopt);
        if (!fatal_error_called) {
            printf("ERROR: expected fatal_error for long string\n");
            return 10;
        }
        
        /** 
        printf("Testing too many commands...\n");
        wgrib2_init_cmds();
        exit_code = 0;
        if (setjmp(exit_jmp) == 0) {
            for (int i = 0; i < 1000; i++) {
                wgrib2_add_cmd("x"); 
            }
            printf("ERROR: wgrib2_add_cmd() did not error on too many options\n");
            return 12;
        } else {
            if (exit_code == 0) {
                printf("ERROR: exit() was called but exit code was not set\n");
                return 13;
            }
        }
        */
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}
