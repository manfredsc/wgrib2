/** @file
 * @brief Converts a float to an IEEE single precision number (big endian).
 * @author Public Domain: Wesley Ebisuzaki @date ????
 */
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "wgrib2.h"

/**
 * Converts a float to an IEEE single precision number (big endian). 
 * 
 * Bugs: 
 * - assumes length of integer >= 25 bits
 * - doesn't handle subnormal numbers
 * 
 * @param x The float value to convert.
 * @param ieee Pointer to the output buffer for the IEEE representation.
 * 
 * @return 0 on success, -1 on error.
 * 
 * @author Wesley Ebisuzaki @date ????
 */
int flt2ieee(float x, unsigned char *ieee) {

    int sign, exp;
    unsigned int umant;
    double mant;

    if (x == 0.0) {
        ieee[0] = ieee[1] = ieee[2] = ieee[3] = 0;
        return 0;
    }

    /* sign bit */
    if (x < 0.0) {
        sign = 128;
        x = -x;
    }
    else sign = 0;
    mant = frexp((double) x, &exp);

    /* 2^24 = 16777216 */

    umant = mant * 16777216 + 0.5;
    if (umant >= 16777216) {
        umant = umant / 2;
        exp++;
    }
    /* bit 24 should be a 1 .. not used in ieee format */

    exp = exp - 1 + 127;

    if (exp < 0) {
        /* signed zero */
        ieee[0] = sign;
        ieee[1] = ieee[2] = ieee[3] = 0;
        return 0;
    }
    if (exp > 255) {
        /* signed infinity */
        ieee[0] = sign + 127;
        ieee[1] = 128;
        ieee[2] = ieee[3] = 0;
        return 0;
    }
    /* normal number */

    ieee[0] = sign + (exp >> 1);

    ieee[3] = umant & 255;
    ieee[2] = (umant >> 8) & 255;
    ieee[1] = ((exp & 1) << 7) + ((umant >> 16) & 127);
    return 0;
}
