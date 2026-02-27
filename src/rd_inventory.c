/** @file
 * @brief Reads inventory and pulls out the address of the record and submessage number.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2005 | W. Ebisuzaki | Initial
 * 10/2014 | W. Ebisuzaki | Moved from wgrib2.c to rd_inventory.c
 *                          update: 10/2014 reads from input rather than stdin
 * 4/2015 | W. Ebisuzaki | Reads into buffer
 * @author Public Domain: Wesley Ebisuzaki @date 2005
 */
#include <stdio.h>
#include <ctype.h>
#include "wgrib2.h"

/** Flag to use fgrep. */
extern int fgrep; 

/** Flag to use fgrep (not used). */
extern int fgrep_flag;

/** Number of fgrep options. */
extern int fgrep_count;
#ifdef USE_REGEX
/** Flag to use egrep. */
extern int egrep;

/** Flag to use egrep (not used). */
extern int egrep_flag;

/** Number of egrep options. */
extern int egrep_count;
#endif

/**
 * Reads inventory and pulls out the address of the record and submessage number.
 * 
 * @param rec_num Pointer to the record number.
 * @param submsg Pointer to the submessage number.
 * @param pos Pointer to the position.
 * @param in Pointer to the input sequence file struct.
 * 
 * @return 0 on success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2005
 */
int rd_inventory(int *rec_num, int *submsg, long int *pos, struct seq_file *in) {

    long unsigned int tmp;
    int c, i;
    int buff_cnt, errmsg;
    char buffer[INV_STRING_SIZE+1], *p;

    /* format: [digits].[submessage]:[pos]: or digits]:[pos]: */

    errmsg = 0;
    while (1) {
        buff_cnt = 0;
        /* read one line */
        c=fgetc_file(in);
        if (c == EOF) return 1;
        while (c != EOF && c != '\n') {
            if (buff_cnt == INV_STRING_SIZE) {
                if (errmsg++ == 0)
                    fprintf(stderr,"rd_inventory: stdin truncated, longer than INV_STRING_SIZE (%d)\n", INV_STRING_SIZE);
            }
            else { 
                buffer[buff_cnt++] = c;
            }
            c = fgetc_file(in);
        }
        buffer[buff_cnt] = 0;

        p = buffer;
        /* skip leading blanks */
        while (*p == ' ') p++;
        if (*p == 0) return 1;

        if (!isdigit((unsigned char) *p)) fatal_error("bad inventory: missing record number","");

        /* record number */
        i = 0;
        while (isdigit((unsigned char) *p) ) {
            i = 10*i + *p++ - '0';
        }
        *rec_num = i;

        /* submessage */
        if (*p == '.') {
            p++;
            i = 0;
            while (isdigit((unsigned char) *p) ) {
                i = 10*i + *p++ - '0';
            }
            *submsg = i;
        }
        else {
            *submsg = 1;
        }

        /* address */

        if (*p++ != ':') fatal_error_i("bad inventory on line: %d",*rec_num);
        tmp = 0;
        while (isdigit((unsigned char) *p)) {
            tmp = 10*tmp + *p++ - '0';
        }
        *pos = tmp;
        if (fgrep == 1) {
        if (is_fgrep(buffer) == 1) continue;
        }
#ifdef USE_REGEX
        if (egrep == 1) {
           if (is_egrep(buffer) == 1) continue;
        }
#endif
        break;
    }
    return 0;
}
