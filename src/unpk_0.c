/** @file
 * @brief Unpack BDS section.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 1996 | W. Ebisuzaki | Initial
 * 7/1998 | W. Ebisuzaki | fix bug for bitmaps and nbit >= 25 found by Larry Brasfield
 * 2/2001 | W. Ebisuzaki | changed jj from long int to double
 * 3/2002 | L. Kornblueh | added unpacking extensions for spectral data, Luis Kornblueh, MPIfM 
 * 7/2006 | W. Ebisuzaki | fixed some bug complex packed data was not set to undefined
 * 10/2015 | W. Ebisuzaki | changed n and i to unsigned
 * 3/2016 | W. Ebisuzaki | OpenMP
 * 6/2016 | W. Ebisuzaki | faster OpenMP and optimization
 * 10/2021 | W. Ebisuzaki | flt[] = (ref0 + bin_scale_packed_int)*dec_scale for more precision
 * @author Public Domain: Wesley Ebisuzaki @date 1996
 */
#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include "wgrib2.h"

#ifdef USE_OPENMP
#include <omp.h>
#else
#define omp_get_num_threads()		1   /**< Number of threads */
#define omp_get_thread_num()		0   /**< Thread ID */
#endif

/** Mask array for unpacking. */
static unsigned int mask[] = {0,1,3,7,15,31,63,127,255};

/** Shift array for unpacking. */
static double shift[9] = {1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0};

/**
 * Unpack BDS section.
 *
 * Code assumes an integer >= 32 bits.
 * 
 * @param flt Pointer to output array.
 * @param bits0 Pointer to packed integer data.
 * @param bitmap0 Pointer to bitmap (undefined data), NULL if none.
 * @param n_bits Number of bits per packed integer.
 * @param n Number of data points (includes undefined data).
 * @param ref0 Reference value.
 * @param bin_scale Binary scale factor.
 * @param dec_scale Decimal scale factor.
 *
 * @author Wesley Ebisuzaki @date 1996
 */
void unpk_0(float *flt, unsigned char *bits0, unsigned char *bitmap0,
	int n_bits, unsigned int n, double ref0, double bin_scale, double dec_scale) {

    unsigned char *bits, *bitmap;

    int c_bits, j_bits, nthreads, thread_id;
    unsigned int map_mask, bbits, i, j, k, n_missing, ndef, di;
    double jj;

//    ref = ref0 * dec_scale;
//    scale = bin_scale * dec_scale;
    bits = bits0;
    bitmap = bitmap0;

    bbits = 0;

    /* assume integer has 32+ bits */
    /* optimized code for n_bits <= 25bits */
    if (n_bits <= 25) {
        n_missing = bitmap ? missing_points(bitmap0, n) : 0;
        ndef = n - n_missing;

	// 1-cpu: rd_bitstream_flt(bits0, 0, flt+n_missing, n_bits, ndef);
	// 1-cpu: for (j = 0; j < ndef; j++) flt[j+n_missing] = ref + scale*flt[j+n_missing];

#ifdef USE_OPENMP
#pragma omp parallel private(i,j,k, nthreads, thread_id)
#endif
        {
            nthreads = omp_get_num_threads();
            thread_id = omp_get_thread_num();

            di = (ndef + nthreads - 1) / nthreads;
            di = ((di + 7) | 7) ^ 7;
            i = thread_id * di;
            if (i < ndef) {
                k  = ndef - i;
                if (k > di) k = di;
                rd_bitstream_flt(bits0 + (i/8)*n_bits, 0, flt+n_missing+i, n_bits, k);
                for (j = i+n_missing; j < i+k+n_missing; j++) {
                    flt[j] = (ref0 + bin_scale*flt[j])*dec_scale;
                }
            }
        }
/*
#pragma omp parallel for private(i,j,k)
	for (i = 0; i < ndef; i += CACHE_LINE_BITS) {
	    k  = ndef - i;
	    if (k > CACHE_LINE_BITS) k = CACHE_LINE_BITS;
	    rd_bitstream_flt(bits0 + (i/8)*n_bits, 0, flt+n_missing+i, n_bits, k);
	    for (j = i+n_missing; j < i+k+n_missing; j++) {
		flt[j] = ref + scale*flt[j];
	    }
	}
*/

        if (n_missing != 0) {
            j = n_missing;
            for (i = 0; i < n; i++) {
                /* check bitmap */
                if ((i & 7) == 0) bbits = *bitmap++;
                flt[i] = (bbits & 128) ?  flt[j++] : UNDEFINED;
                bbits = bbits << 1;
            }
        }
    }
    else {
        /* older unoptimized code, not often used */
        c_bits = 8;
        map_mask = 128;
        while (n-- > 0) {
            if (bitmap) {
                j = (*bitmap & map_mask);
                if ((map_mask >>= 1) == 0) {
                    map_mask = 128;
                    bitmap++;
                }
                if (j == 0) {
                    *flt++ = UNDEFINED;
                    continue;
                }
            }

            jj = 0.0;
            j_bits = n_bits;
            while (c_bits <= j_bits) {
                if (c_bits == 8) {
                    jj = jj * 256.0  + (double) (*bits++);
                    j_bits -= 8;
                }
                else {
                    jj = (jj * shift[c_bits]) + (double) (*bits & mask[c_bits]);
                    bits++;
                    j_bits -= c_bits;
                    c_bits = 8;
                }
            }
            if (j_bits) {
                c_bits -= j_bits;
                jj = (jj * shift[j_bits]) + (double) ((*bits >> c_bits) & mask[j_bits]);
            }
            *flt++ = (ref0 + bin_scale*jj)*dec_scale;
        }
    }
    return;
}
