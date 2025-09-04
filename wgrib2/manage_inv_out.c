/** @file
 * @brief Manage the output buffer for the inventory.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2019
 */
#include <stdio.h>
#include "wgrib2.h"

/** Pointer to current inventory output buffer. */
char *inv_out;

/** Pointer to last inventory output buffer. */
char *last_inv_out;

/** First private inventory output buffer. */
static char inv_buf1[INV_BUFFER];

/** Second private inventory output buffer. */
static char inv_buf2[INV_BUFFER];

/* initialize once */

/**
 * Initialize the inventory output buffers.
 *
 * @author Wesley Ebisuzaki @date 12/2019
 */
void init_inv_out(void) {
    inv_buf1[0] = inv_buf2[0] = '\0';
    inv_out = &(inv_buf1[0]);
    last_inv_out = &(inv_buf2[0]);
}

/**
 * Switch to a new inventory output buffer.
 *
 * @author Wesley Ebisuzaki @date 12/2019
 */
void new_inv_out(void) {
    if (inv_out == &(inv_buf1[0])) {
        inv_out = &(inv_buf2[0]);
        last_inv_out = &(inv_buf1[0]);
        inv_buf2[0] = '\0';
    }
    else if (inv_out == &(inv_buf2[0])) {
        inv_out = &(inv_buf1[0]);
        last_inv_out = &(inv_buf2[0]);
        inv_buf1[0] = '\0';
    }
    else
        fatal_error("new_inv_bufr: programming error","");
    return;
}

/**
 * Repeat the last inventory output buffer.
 *
 * @author Wesley Ebisuzaki @date 12/2019
 */
void repeat_inv_out(void) {		// allow last/last0 to keep old inv_buf
    if (inv_out == &(inv_buf1[0])) {
        inv_out = &(inv_buf2[0]);
        last_inv_out = &(inv_buf1[0]);
    }
    else if (inv_out == &(inv_buf2[0])) {
        inv_out = &(inv_buf1[0]);
        last_inv_out = &(inv_buf2[0]);
    }
    else
        fatal_error("new_inv_bufr: programming error","");
    return;
}

/**
 * Get the current inventory output buffer.
 *
 * @return Pointer to the current inventory output buffer.
 *
 * @author Wesley Ebisuzaki @date 12/2019
 */
char *base_inv_out(void) {
    return inv_out;
}
