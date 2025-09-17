/** @file
 * @brief Return string with short format integer.
 * @author Public Domain: Wesley Ebisuzaki @date 10/2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"

/**
 * Convert integer to short format string.
 * 
 * @param string Pointer to the output string.
 * @param i Integer to convert.
 * 
 * @author Wesley Ebisuzaki @date 10/2013
 */
void itoshort_a(char *string, int i) {
    int itmp, exp;

    if (i == 0) {
        strcpy(string, "0");
        return;
    }

    if (i < 0) {
        *string++ = 'n';
        i = -i;
    }

    exp = 0;
    itmp = i;
    while (i % 10 == 0) { i /=  10; exp++; }
    if (exp >= 3) sprintf(string, "%de%d", i, exp);
    else sprintf(string, "%d", itmp);
    return;
}

