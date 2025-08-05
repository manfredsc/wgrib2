/** @file
 * @brief Bitstream handling functions for wgrib2.
 * @author Public Domain: Wesley Ebisuzaki @date 6/2009
 */

#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include "wgrib2.h"

/** Array of masks for extracting bits */
static unsigned int ones[]={0, 1,3,7,15, 31,63,127,255};

/**
 * Reads a bitstream and converts it to an array of unsigned integers.
 * 
 * @param p Pointer to the start of the bitstream.
 * @param offset Bit offset to start reading from (0-7).
 * @param u Pointer to the output array of unsigned integers.
 * @param n_bits Number of bits to read for each integer. 
 * @param n Number of integers to read.
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 6/2009 | W. Ebisuzaki | Initial
 * 04/2022 | W. Ebisuzaki | limit to nbits is now 32 (32 bit integer), 
 *                          rd_bitstream_offset -> rd_bitstream
 * 
 * @note The function assumes that the bitstream starts on a byte boundary. 
 * For 32-bit machines, n_bits should be less than or equal to 25.
 * 
 * bitstream (n_bits/uint) -> u[0..n-1]
 * 
 * @author Wesley Ebisuzaki @date 6/2009
 */
void rd_bitstream(unsigned char *p, int offset, int *u, int n_bits, unsigned int n) {

    unsigned int tbits;
    int t_bits, new_t_bits;
    unsigned int i;

    // not the best of tests

    if (INT_MAX <= 2147483647 && n_bits > 31)
                fatal_error_i("rd_bitstream: n_bits is %d", n_bits);

    if (offset < 0 || offset > 7) fatal_error_i("rd_bitstream: illegal offset %d",offset);

    if (n_bits == 0) {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
        for (i = 0; i < n; i++) {
            u[i] = 0;
        }
        return;
    }

    t_bits = 8 - offset;
    tbits = (*p++) & ones[t_bits];

    for (i = 0; i < n; i++) {

        while (n_bits - t_bits >= 8) {
            t_bits += 8;
            tbits = (tbits << 8) | *p++;
        }

        if (n_bits > t_bits) {
            new_t_bits = 8 - (n_bits - t_bits);
            u[i]  = (int) ( (tbits << (n_bits - t_bits) | (*p >> new_t_bits) ));
            t_bits = new_t_bits;
            tbits = *p++ & ones[t_bits];
        }
        else if (n_bits == t_bits) {
            u[i]  = (int) tbits;
            tbits = t_bits = 0;
        }
        else {
            t_bits -= n_bits;
            u[i] = (int) (tbits >> t_bits);
            tbits = tbits & ones[t_bits];
        }
    }
}

/**
 * Reads a bitstream and converts it to an array of floats.
 * 
 * @param p Pointer to the start of the bitstream.
 * @param offset Bit offset to start reading from (0-7).
 * @param u Pointer to the output array of floats.
 * @param n_bits Number of bits to read for each float.
 * @param n Number of floats to read.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 6/2009 | W. Ebisuzaki | Initial
 * 04/2022 | W. Ebisuzaki | limit to nbits is now 32 (32 bit integer), 
 *                          rd_bitstream_offset -> rd_bitstream
 * 
 * @author Wesley Ebisuzaki @date 6/2009
 */
void rd_bitstream_flt(unsigned char *p, int offset, float *u, int n_bits, unsigned int n) {

    unsigned int tbits;
    int t_bits, new_t_bits;
    unsigned int i;
    // not the best of tests

    if (INT_MAX <= 2147483647 && n_bits > 31)
                fatal_error_i("rd_bitstream: n_bits is %d", n_bits);

    if (offset < 0 || offset > 7) fatal_error_i("rd_bitstream_flt: illegal offset %d",offset);

    if (n_bits == 0) {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
        for (i = 0; i < n; i++) {
            u[i] = 0.0;
        }
        return;
    }

    t_bits = 8 - offset;
    tbits = (*p++) & ones[t_bits];

    for (i = 0; i < n; i++) {

        while (n_bits - t_bits >= 8) {
            t_bits += 8;
            tbits = (tbits << 8) | *p++;
        }

        if (n_bits > t_bits) {
            new_t_bits = 8 - (n_bits - t_bits);
            u[i]  = (int) ( (tbits << (n_bits - t_bits) | (*p >> new_t_bits) ));
            t_bits = new_t_bits;
            tbits = *p++ & ones[t_bits];
        }
        else if (n_bits == t_bits) {
            u[i]  = (float) tbits;
            tbits = t_bits = 0;
        }
        else {
            t_bits -= n_bits;
            u[i] = (float) (tbits >> t_bits);
            tbits = tbits & ones[t_bits];
        }
    }
}

/** Pointer to bitstream. */
static unsigned char *bitstream;

/** Number of bits remaining in the current byte. */
static int rbits;

/** Bit accumulator for the current byte. */
static int reg;

/** Total number of bits written to the bitstream. */
static int n_bitstream;

/** 
 * Write integer to the bitstream with a specified number of bits.
 * 
 * @param t Integer to add to the bitstream.
 * @param n_bits Number of bits to represent the integer. Should be less than or equal to 25.
 * 
 * @note Bitstream is initialized with init_bitstream(). To close the bitstream, call finish_bitstream().
 * 
 * @author Wesley Ebisuzaki @date 6/2009
*/
void add_bitstream(int t, int n_bits) {
    unsigned int jmask;

    if (n_bits > 16) {
        add_bitstream(t >> 16, n_bits - 16);
        n_bits = 16;
    } 
    if (n_bits > 25) fatal_error_i("add_bitstream: n_bits = (%d)",n_bits);
    jmask = (1 << n_bits) - 1;
    rbits += n_bits;
    reg = (reg << n_bits) | (t & jmask);
    while (rbits >= 8) {
        *bitstream++ = (reg >> (rbits = rbits-8)) & 255;
        n_bitstream++;
    }
    return;
}

/**
 * Write multiple integers to the bitstream with a specified number of bits.
 * 
 * @param t Pointer to the array of integers to add to the bitstream.
 * @param n Number of integers to write.
 * @param n_bits Number of bits to represent each integer. Should be less than or equal to 25.
 * 
 * @note Bitstream is initialized with init_bitstream(). To close the bitstream, call finish_bitstream().
 * 
 * @author Wesley Ebisuzaki @date 6/2009
 */
void add_many_bitstream(int *t, unsigned int n, int n_bits) {
    unsigned int jmask, tt;
    unsigned int i;

    if (n_bits > 25) fatal_error_i("add_many_bitstream: n_bits = (%d)",n_bits);
    jmask = (1 << n_bits) - 1;

    for (i = 0; i < n; i++) {
        tt = (unsigned int) *t++;
        rbits += n_bits;
        reg = (reg << n_bits) | (tt & jmask);

        while (rbits >= 8) {
            rbits -= 8;
            *bitstream++ = (reg >> rbits) & 255;
            n_bitstream++;
        }

/*

        if (rbits == 32) {
	    rbits = 0;
	    ss = reg;
	    bitstream[0] = (ss >> 24) & 255;
	    bitstream[1] = (ss >> 16) & 255;
	    bitstream[2] = (ss >> 8) & 255;
	    bitstream[3] = ss & 255;
	    bitstream += 4;
            n_bitstream += 4;
	}
	else if (rbits >= 24) {
	    rbits = rbits - 24;
	    ss = reg >> rbits;
	    bitstream[0] = (ss >> 16) & 255;
	    bitstream[1] = (ss >> 8) & 255;
	    bitstream[2] = ss & 255;
	    bitstream += 3;
            n_bitstream += 3;
	}
	else if (rbits >= 16) {
	    rbits = rbits - 16;
	    ss = reg >> rbits;
	    bitstream[1] = ss & 255;
	    bitstream[0] = (ss >> 8) & 255;
	    bitstream += 2;
            n_bitstream += 2;
	}
	else if (rbits >= 8) {
	    rbits = rbits-8;
	    *bitstream++ = (reg >> rbits) & 255;
            n_bitstream++;
	}
*/
    }
    return;
}

/**
 * Initialize the bitstream with a given buffer.
 * 
 * @param new_bitstream Pointer to the buffer where the bitstream will be stored.
 * 
 * @note This function should be called before adding any bits to the bitstream. 
 * To close the bitstream, call finish_bitstream().
 * 
 * @author Wesley Ebisuzaki @date 6/2009
 */
void init_bitstream(unsigned char *new_bitstream) {
    bitstream = new_bitstream;
    n_bitstream = reg = rbits = 0;
    return;
}

/**
 * Close the bitstream by writing any remaining bits to the buffer. Last byte is zero packed.
 * 
 * @note This function should be called after all bits have been added to the bitstream.
 * 
 * @author Wesley Ebisuzaki @date 6/2009
 */
void finish_bitstream(void) {
    if (rbits) {
        n_bitstream++;
        *bitstream++ = (reg << (8-rbits)) & 255;
        rbits = 0;
    }
    return;
}
