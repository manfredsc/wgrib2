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

        printf("    Test where exp < 0.\n");
        in_x = -ldexpf(1.0f, -128);
        flt2ieee(in_x, buf);
        out_x = ieee2flt(buf);
        if (in_x != out_x) return 11;
    }
}