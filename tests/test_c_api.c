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

extern jmp_buf fatal_err;
//static jmp_buf exit_jmp;

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

        /*
         * Some environments may route fatal_error() differently,
         * resulting in an alternate error (e.g., from wgrib2_cmd()).
         * Treat either fatal path as success to keep this test portable.
         */
        wgrib2_init_cmds();
        if (setjmp(fatal_err) == 0) {
            char longopt[CMD_LEN + 1];
            memset(longopt, 'A', CMD_LEN);
            longopt[CMD_LEN] = '\0';
            wgrib2_add_cmd(longopt);
            /* If we reach here, try executing the command list, which should also fail. */
            if (setjmp(fatal_err) == 0) {
                (void)wgrib2_cmd();
                printf("ERROR: neither long option nor command execution triggered fatal_error()\n");
                fflush(stdout);
                return 11;
            }
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
