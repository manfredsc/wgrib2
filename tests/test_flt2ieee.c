/**
 * This tests the functions converting between ieee and float.
 * 
 * Includes flt2ieee(), flt2ieee_nan(), ieee2flt(), and ieee2ieee_nan()
 * 
 * Alyson Stahl, 12/2025
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"

int
main(){
    printf("Testing flt2ieee() and ieee2flt()...\n");
    {
        float in_x, out_x;
        unsigned char buf[4];

        printf("    Test with x = 0.0\n");
        in_x = 0.0;
        flt2ieee(in_x, buf);
        out_x = ieee2flt(buf);
        if (in_x != out_x) return 10;

        printf("    Test with x < 0.\n");
        in_x = -1.9999999;
        flt2ieee(in_x, buf);
        out_x = ieee2flt(buf);
        if (in_x != out_x) return 11;

        printf("    Test with x > 0.\n");
        in_x = 1.9999999;
        flt2ieee(in_x, buf);
        out_x = ieee2flt(buf);
        if (in_x != out_x) return 12;

        printf("    Test where exp < 0.\n");
        in_x = ldexpf(1.0f, -128);
        flt2ieee(in_x, buf);
        out_x = ieee2flt(buf);
        if (out_x != 0.0f) return 13;

        printf("    Test with x = infinity\n");
        in_x = INFINITY;
        flt2ieee(in_x, buf);
        if (buf[0] != 0x7F || buf[1] != 0x80 || buf[2] != 0x00 || buf[3] != 0x00) return 14;
    }
    printf("ok!\n");
    printf("Testing flt2ieee_nan() and ieee2flt_nan()...\n");
    {
        float in_x, out_x;
        unsigned char buf[4];

        printf("    Test with x = 0.0\n");
        in_x = 0.0;
        flt2ieee_nan(in_x, buf);
        out_x = ieee2flt_nan(buf);
        if (in_x != out_x) return 10;

        printf("    Test with x < 0.\n");
        in_x = -1.9999999;
        flt2ieee_nan(in_x, buf);
        out_x = ieee2flt_nan(buf);
        if (in_x != out_x) return 11;

        printf("    Test with x > 0.\n");
        in_x = 1.9999999;
        flt2ieee_nan(in_x, buf);
        out_x = ieee2flt_nan(buf);
        if (in_x != out_x) return 12;

        printf("    Test where exp < 0.\n");
        in_x = ldexpf(1.0f, -128);
        flt2ieee_nan(in_x, buf);
        out_x = ieee2flt_nan(buf);
        if (out_x != 0.0f) return 13;

        printf("    Test with x = infinity\n");
        in_x = INFINITY;
        flt2ieee_nan(in_x, buf);
        if (buf[0] != 0x7F || buf[1] != 0x80 || buf[2] != 0x00 || buf[3] != 0x00) return 14;

        printf("    Test with x = NaN\n");
        in_x = NAN;
        flt2ieee_nan(in_x, buf);
        out_x = ieee2flt_nan(buf);
        if (!isnan(out_x)) return 15;
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}