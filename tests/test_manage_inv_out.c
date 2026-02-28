/**
 * This is a test for the wgrib2 project. This test is for manage_inv_out.c.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include <setjmp.h>
#include "wgrib2.h"

extern jmp_buf fatal_err;
extern char *inv_out;
extern char *last_inv_out;

int
main()
{
    int i;
    const char *test_string = "some string";

    char *buf = NULL;
    char *buf1 = NULL;
    char *buf2 = NULL;

    printf("Testing new_inv_out() without calling init_inv_out(). "
        "Should throw a fatal_error()...\n");

    if (setjmp(fatal_err) == 0) {
        new_inv_out();
        return 10;
    }

    printf("Testing repeat_inv_out() without calling init_inv_out(). "
        "Should throw a fatal_error()...\n");

    if (setjmp(fatal_err) == 0) {
        repeat_inv_out();
        return 11;
    }

    printf("Testing simple call of init_inv_out()...\n");
    init_inv_out();
    if (inv_out == NULL || last_inv_out == NULL) {
        return 12;
    }
    if (inv_out == last_inv_out) {
        return 13;
    }

    /* Write something to inv_out */
    inv_out[0] = 'X';
    inv_out[1] = '\0';
    for (i = 0; test_string[i] != '\0'; i++) {
        inv_out[i] = test_string[i];
    }
    inv_out[i] = '\0';

    printf("Testing base_inv_out(). Should return the current inv_out buffer...\n");
    buf = base_inv_out();
    if (buf != inv_out) {
        return 14;
    }

    printf("Testing initial call of new_inv_out(). Should switch to a new buffer...\n");
    new_inv_out();
    if (inv_out == buf) {
        return 15;
    }

    /* Capture buffer pointers after first new_inv_out() */
    buf1 = last_inv_out;
    buf2 = inv_out;

    printf("Calling new_inv_out() again. Should switch to a new buffer...\n");
    new_inv_out();
    if (inv_out != buf1 || last_inv_out != buf2) {
        return 16;
    }

    printf("Testing initial call of repeat_inv_out(). Should switch to the previous buffer...\n");
    repeat_inv_out();
    if (inv_out != buf2 || last_inv_out != buf1) {
        return 17;
    }

    printf("Calling repeat_inv_out() again. Should switch to the previous buffer...\n");
    repeat_inv_out();
    if (inv_out != buf1 || last_inv_out != buf2) {
        return 18;
    }

    /* Verify that buf1 still contains test_string */
    for (i = 0; test_string[i] != '\0'; i++) {
        if (buf1[i] != test_string[i]) {
            return 19;
        }
    }
    if (buf1[i] != '\0') {
        return 19;
    }

    printf("Testing second call to init_inv_out(). Should reset both buffers to empty strings...\n");
    init_inv_out();
    if (inv_out != buf1 || last_inv_out != buf2) {
        return 20;
    }
    if (inv_out[0] != '\0' || last_inv_out[0] != '\0') {
        return 21;
    }

    printf("SUCCESS!\n");
    return 0;
}