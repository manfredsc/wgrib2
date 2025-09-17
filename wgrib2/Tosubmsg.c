/** @file
 * @brief Write submessages.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 6/2009 | R.N. Bokhorst| Initial
 * 6/2009 | W. Ebisuzaki | Some changes
 * @author Public Domain: R.N. Bokhorst, reinoud.bokhorst@bmtargoss.com @date 6/2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

// #define DEBUG

/** Get size of GRIB2 section i.*/
#define GB2_Sec_i_size(i)  (uint4(sec[i]+0))

/** Struct to hold submessage information. */
struct submsg {
    long start_pos;                 /**< Start position in output file */
    size_t saved_space;             /**< Amount of space saved */
    size_t written_count;           /**< Number of written messages */
    size_t written_bytes;           /**< Number of written bytes */
    unsigned char *last_sec[9];     /**< Last section data */
    struct seq_file out;            /**< Output file */
};

/** Append grib file flag. */
extern int file_append;

/** Flush of output flag. */
extern int flush_mode;

/* mode functions */
static int init_tosubmsg(ARG1, struct submsg *save);
static int write_tosubmsg(ARG1, struct submsg *save);
static int cleanup_tosubmsg(ARG1, struct submsg *save);

/* check for same section .. only for sections 1..7 */

/**
 * Check if two GRIB2 sections are the same. Only checks sections 1 to 7.
 *
 * @param seca Pointer to the first GRIB section.
 * @param secb Pointer to the second GRIB section.
 *
 * @return 1 if the sections are the same, 0 otherwise.
 *
 * @author R.N. Bokhorst @date 6/2009
 */
static int same_sec(unsigned char *seca, unsigned char *secb) {
    unsigned int a, b, i;
    if (seca == NULL && secb == NULL) return 1;
    if (seca == NULL || secb == NULL) return 0;
    a = uint4(seca);
    b = uint4(secb);
    if (a != b) return 0;
    for (i = 0; i < a; i++) {
        if (seca[i] != secb[i]) return 0;
    }
    return 1;
}

/*
 * HEADER:100:tosubmsg:output:1:convert GRIB message to submessage and write to file X
 */

/**
 * Writes out the current field as a submessage in a larger grib message.
 * 
 * This can save space as various sections do not have to be repeated. For example, often you 
 * only need one grid definition section for all the submessages. This feature can save space 
 * when the bitmaps are repeated. By combining the fields into one message, only one copy of 
 * the bitmap is needed. The -tosubmsg option will only write out one grib message. Fields that 
 * cannot be added to the grib message will be ignored with warning. Fields cannot be added to 
 * the the submessage if section 0 (except the message length) or section 1 differ. 
 * 
 * Note that the -tosubmsg is the only grib output option which cannot write to a pipe. However, 
 * the somewhat similar -ncep_uv option is pipe compatible. 
 * 
 * By combining several grib messages into one big message, you can save space. The practical 
 * drawbacks include 
 * 
 * - A few codes cannot handle submessages
 * - Decoders tend to load the entire grib message into memory. Loading a 2+ GB grib message 
 * would be a problem on 32-bit machines and machines with limited memory.
 * - Inefficient with "partial-http downloading" (of concern to web servers)
 * 
 * However, submessages can be very important for some grids. One grid template stores the 
 * latitude and longitude of each grid point. This is a large overhead per grib message but by 
 * using submessages, the total overhead is reduced. 
 * 
 * ## Usage
 * -tosubmsg OUTPUT_FILE
 * 
 * NOTE: OUTPUT_FILE cannot be a pipe.
 * 
 * @param ARG1 ???
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 *
 * @author R.N. Bokhorst @date 6/2009
 */
int f_tosubmsg(ARG1) {
    struct submsg *save;

    if (mode == -1) {
        *local = save = (struct submsg *) malloc( sizeof(struct submsg));
        if (save == NULL) fatal_error("memory allocation tosubmsg","");
        init_tosubmsg(call_ARG1(inv_out,local,arg1), save);
    }
    else if (mode >= 0) {
        save = (struct submsg *) *local;
        write_tosubmsg(call_ARG1(inv_out,local,arg1), save);
    }
    else if (mode == -2) {
        save = (struct submsg *) *local;
        cleanup_tosubmsg(call_ARG1(inv_out,local,arg1), save);
    }
    return 0;
}

/**
 * Initialize the submessage structure (mode = -1).
 * 
 * @param ARG1 ???
 * @param save Pointer to the submessage structure.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author R.N. Bokhorst @date 6/2009
 */
int init_tosubmsg(ARG1, struct submsg *save) {

    /* Open output file */
    /* Note that we have to use rb+ instead of ab in append mode, otherwise we cannot
       overwrite section 0 somewhere in the middle of the file */

    if (fopen_file(&(save->out), arg1, file_append ? "rb+" : "wb") != 0) {
        free(save);
        fatal_error("Could not open %s", arg1);
    }
    if (file_append) {
        /* rb+ mode positions file at the beginning */
        fseek_file(&(save->out), 0L, SEEK_END);
    }
    if (save->out.file_type == PIPE) fatal_error("tosubmsg: does not work with pipes %s", arg1);

    /* save start position in file for rewriting section 0 */
    save->start_pos = ftell_file(&(save->out));
    save->saved_space = 0;
    save->written_count = 0;
    save->written_bytes = 0;
    init_sec(save->last_sec);

    return 0;
}

/**
 * Process each (sub)message (mode = 0,1,2).
 * 
 * @param ARG1 ???
 * @param save Pointer to the submessage structure.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author R.N. Bokhorst @date 6/2009
 */
int write_tosubmsg(ARG1, struct submsg *save) {
    int i, ok;
    static unsigned char sec6_repeat[] = {0,0,0,6,6,254};

    if (save->written_count == 0L) {

        /* first message */

        /* copy all sections */
        copy_sec(sec, save->last_sec);

        /* write sections 0..7 */

        fwrite_file((void *) sec[0], sizeof(char), GB2_Sec0_size, &(save->out));
        save->written_bytes = (size_t) GB2_Sec0_size;

        for (i = 1; i <= 7; i++) {
            if (sec[i]) {
                fwrite_file((void *) sec[i], sizeof(char), GB2_Sec_i_size(i), &(save->out));
                save->written_bytes += GB2_Sec_i_size(i);
            }
        }
        save->written_count++;
        return 0;
    }

    /* can only merge if sec0 and sec1 are the same */

    ok = 1;
    // check discipline
    if (sec[0][6] != save->last_sec[0][6]) ok = 0;
    // check grib number
    if (sec[0][7] != save->last_sec[0][7]) ok = 0;

    if (same_sec(sec[1],save->last_sec[1]) == 0) ok = 0;

    if (ok == 0) {
        fprintf(stderr,"tosubmsg: only handle one discipline at a time, record not saved\n");
        return 0;
    }

    ok = 1;  	// ok to not to write
    for (i = 2; i <= 7; i++) {
        if (i == 4) ok = 0;	// can only skip sections 2 or 3
        if (ok == 1) {
            if (same_sec(sec[i],save->last_sec[i]) == 0) ok = 0;
        }
        if (ok == 0) {
            if (i == 6 && GB2_Sec6_size(sec) > 6 && same_sec(sec[i],save->last_sec[i])) {
                // if same bitmap as before .. use special code
                fwrite_file(sec6_repeat, sizeof(char), 6, &(save->out));
                save->written_bytes += 6;
                save->saved_space += GB2_Sec6_size(sec) - 6;
                if (mode == 99) fprintf(stdout, ":Bitmap indicator set to 254");
            }
            else {
                if (sec[i] != NULL) {
                    fwrite_file((void *) sec[i], sizeof(char), GB2_Sec_i_size(i), &(save->out));
                    save->written_bytes += GB2_Sec_i_size(i);
                }
            }
        }
        else {
            // save space by not writing out a duplicate section
            if (sec[i]) save->saved_space += GB2_Sec_i_size(i);
        }
    }

    // refresh the sections
    // this could be optimized
    free_sec(save->last_sec);
    copy_sec(sec, save->last_sec);

    save->written_count++;

    return 0;
}

/**
 * Cleanup the submessage structure (mode = -2).
 * 
 * @param ARG1 ???
 * @param save Pointer to the submessage structure.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author R.N. Bokhorst @date 6/2009
 */
static int cleanup_tosubmsg(ARG1, struct submsg *save) {

    unsigned char s[GB2_Sec8_size];

    if (save->written_count > 0) {
        /* Write section 8 */
        s[0] = s[1] = s[2] = s[3] = 55; /* 7777 */
        fwrite_file((void *) s, sizeof(char), GB2_Sec8_size, &(save->out));
        save->written_bytes += (size_t) GB2_Sec8_size;

        /* Rewrite section 0 with correct total size */
        fseek_file(&(save->out), save->start_pos, SEEK_SET);
        uint8_char(save->written_bytes, save->last_sec[0]+8);
        fwrite_file(save->last_sec[0], sizeof(char), GB2_Sec0_size, &(save->out));
    }


    fprintf(stderr, "\nSubmessage statistics:\n"
                  "- # submessages written  : %ld\n"
                  "- Kbytes saved           : %ld\n"
                  "- Kbytes written         : %ld\n"
                        ,save->written_count, save->saved_space/1024, save->written_bytes/1024);

    fclose_file(&(save->out));
    free_sec(save->last_sec);
    return 0;
}
