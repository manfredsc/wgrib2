/** @file
 * @brief Integer power function.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 12/2008 | W. Ebisuzaki | Initial implementation
 * 02/2021 | W. Ebisuzaki | Converted y to unsigned int p
 *                          a future compiler may complain about bit operations on ints
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 12/2008
 */

#include <stdio.h>
#include "wgrib2.h"

/**
 * Integer power function.
 * 
 * @param x Base value.
 * @param y Exponent value.
 * 
 * @return x ** y
 * 
 * @author Wesley Ebisuzaki @date 12/2008
 */
double Int_Power(double x, int y) {

    double value;
    unsigned int p;

    if (y < 0) {
        p = -y;
        x = 1.0 / x;
    }
    else p = y;

    value = 1.0;

    while (p) {
        if (p & 1) value *= x;
        x = x * x;
        p >>= 1;
    }
    return value;
}
