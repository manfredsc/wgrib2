/**
 * This tests the the functions in flt2ieee.c and flt2ieee_nan.c.
 * 
 * Alyson Stahl, 12/2025
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"

int
main(){
    printf("Testing flt2ieee()...\n");
    {
        float x;
        unsigned char buf[4], exp_buf[4];
        
        printf("    Test with x = 0.0\n");
        x = 0.0;
        flt2ieee(x, buf);
        exp_buf[0] = 0x00;
        exp_buf[1] = 0x00;
        exp_buf[2] = 0x00;
        exp_buf[3] = 0x00;
        if (memcmp(buf, exp_buf, 4) != 0) return 10;

        printf("    Test case where exp < 0\n");
        x = -ldexpf(1.0f, -129);
        flt2ieee(x, buf);
        exp_buf[0] = 0x80;
        if (memcmp(buf, exp_buf, 4) != 0) return 11;

        //printf("    Test case where exp > 255\n");
        //x = ldexpf(1.0f, 129);
        //flt2ieee(x, buf);
        //exp_buf[0] = 0x7f;
        //exp_buf[1] = 0x80;
        //exp_buf[2] = 0x00;
        //exp_buf[3] = 0x00;
        //if (memcmp(buf, exp_buf, 4) != 0) return 12;
    }
}