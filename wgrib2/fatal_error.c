/** @file
 * @brief Writes fatal error messages so that they have a common format.
 * @author Public Domain: Wesley Ebisuzaki @date 2004
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2004 | W. Ebisuzaki | Initial
 * 2021 | G. Trojan | Added fatal_error_XY, code for callable wgrib2, calls to err_bin, err_string
 * ???? | G. Trojan | Replace various fatal_error*(..) by one routine using vfprintf(..) 
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <setjmp.h>

#include "wgrib2.h"

/** Stores fatal error. */
extern jmp_buf fatal_err;

/**
 * Writes a fatal error message. 
 * 
 * fatal_error(ARGS) is replacement for
 *         fprintf(ARGS)
 *         do_fatal_error_processing
 * 
 * @param fmt Format string for the error message.
 * @param ... Additional arguments for the format string.
 * 
 * @author Wesley Ebisuzaki @date 2004
 */
void fatal_error(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    fprintf(stderr, "\n*** FATAL ERROR: ");
    vfprintf(stderr, fmt, arg);
    fprintf(stderr," ***\n\n");
    va_end(arg);

    err_bin(1); err_string(1);

    longjmp(fatal_err,1);

    exit(8);
    return;
}
