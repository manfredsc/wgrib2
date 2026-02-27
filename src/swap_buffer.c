/** @file
 * @brief Does a byte swap of 4-byte integers/ieee.
 * 
* ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2008 | W. Ebisuzaki | Initial
 * 7/2015 | W. Ebisuzaki | OpenMP version
 * @author Public Domain: Wesley Ebisuzaki @date 3/2008
 */

#include <stdio.h>
#include "wgrib2.h"

/**
 * Swaps the byte order of a 4-byte integers/ieee.
 * 
 * @param buffer Pointer to the buffer containing the data to swap.
 * @param n The number of bytes in the buffer (must be a multiple of 4).
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int swap_buffer(unsigned char *buffer, unsigned int n) {
    unsigned int ii;
    unsigned char i, j;

#ifdef USE_OPENMP
#pragma omp parallel for private(ii, i, j)
#endif
    for (ii = 0; ii < n; ii += 4) {
        i = buffer[ii];
        j = buffer[ii+1];
        buffer[ii] = buffer[ii+3];
        buffer[ii+1] = buffer[ii+2];
        buffer[ii+2] = j;
        buffer[ii+3] = i;
    }
    return 0;
}


