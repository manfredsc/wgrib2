/** @file
 * @brief Write IEEE file - big endian format.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 7/1997 | W. Ebisuzaki | Initial
 * 7/1997 | W. Ebisuzaki | Buffered, faster
 * 2/1999 | W. Ebisuzaki | Fixed (typo) error in wrtieee_header found by Bob Farquhar
 * 3/2008 | W. Ebisuzaki | Added little-endian output
 * 11/2013 | W. Ebisuzaki | Removed h4[] to cleanup not initialized warning
 * 7/2015 | W. Ebisuzaki | OpenMP support, write to fwrite_file, bigger buffer
 * 12/2017 | W. Ebisuzaki | Size(float) -> 4
 * @author Public Domain: Wesley Ebisuzaki @date 7/1997
 */

#include <stdio.h>
#include <stddef.h>
#include "wgrib2.h"

/** Buffer size for IEEE writing - must be a multiple of 4. */
#define BSIZ (64u*1024u*4u)

/** IEEE little endian flag. */
extern int ieee_little_endian;

/**
 * Write IEEE file - big endian format.
 * 
 * @param array Pointer to the data array.
 * @param n Number of elements in the array.
 * @param header Header flag (1 for F77 style header, 0 for none).
 * @param out Pointer to the output file.
 *
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 7/1997
 */
int wrtieee(float *array, unsigned int n, int header, struct seq_file *out) {

    unsigned int i, j, l, nbuf, loop;
    unsigned char buff[BSIZ];

    nbuf = 0;
    if (header) {
        if (n >= 4294967295U / 4) 	// size(ieee) == 4
            fatal_error("wrtieee: grid too large for 4-byte header","");
        l = n * 4;

        buff[nbuf  ] = (l >> 24) & 255;
        buff[nbuf+1] = (l >> 16) & 255;
        buff[nbuf+2] = (l >>  8) & 255;
        buff[nbuf+3] = l         & 255;
        nbuf += 4;
    }
    i = 0;
    while (i < n) {
        loop = (BSIZ - nbuf)/4;
        loop  = (n-i) > loop ? loop : (n-i);
#ifdef USE_OPENMP
#pragma omp parallel for private(j) schedule(static)
#endif
        for (j = 0 ; j < loop; j++) {
            flt2ieee(array[i+j], buff + nbuf + j*4);
        }
        i += loop;
        nbuf += 4*loop;

        if (nbuf >= BSIZ) {		// nbuf should never be > BSIZ
            if (ieee_little_endian) swap_buffer(buff, BSIZ);
            fwrite_file(buff, 1, BSIZ, out);
            nbuf = 0;
        }
    }
    if (header) {
        l = n * 4;
        buff[nbuf  ] = (l >> 24) & 255;
        buff[nbuf+1] = (l >> 16) & 255;
        buff[nbuf+2] = (l >>  8) & 255;
        buff[nbuf+3] = l         & 255;
        nbuf += 4;
    }
    if (nbuf) {
        if (ieee_little_endian) swap_buffer(buff, nbuf);
        fwrite_file(buff, 1, nbuf, out);
    }
    return 0;
}
