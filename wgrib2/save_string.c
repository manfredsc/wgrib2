/** @file
 * @brief When you read a table from a file, you want to setup structures.
 *
 * Strings are a problem because the size changes. This package allows you to save a string 
 * in memory.
 * @author Public Domain: Wesley Ebisuzaki @date 3/2012
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"

/** Current size of the string buffer. */
static int csize = 0;

/** Maximum size of the string buffer. */
static int csize_max = 0;

/** Pointer to the string buffer. */
static char *cbuffer = NULL;

/**
 * Save strings in a dynamic memory buffer.
 *
 * @param string Pointer to the string to save.
 *
 * @return Pointer to the saved string in the buffer.
 *
 * @author Wesley Ebisuzaki @date 3/2012
 */
char *save_string(char *string) {
    int len;
    char *cbuffer_new;

    len = strlen(string);

    while (len + csize + 1 > csize_max) {
        /*  need to add more memory */
        if (csize_max == 0) {
            csize_max = EXT_TABLE_SIZE;
            cbuffer = malloc(csize_max);
        }
        else {
            csize_max += csize_max;
            cbuffer_new = realloc(cbuffer, csize_max);
            if (cbuffer_new == NULL) {
                free(cbuffer);
                fatal_error_i("save_string: memory allocation failed, %d bytes", csize);
            }
            else {
                cbuffer = cbuffer_new;
            }
        }
    }
    strncpy(cbuffer+csize,string,len+1);
    csize += len + 1;
    return cbuffer + csize - len - 1;
}
