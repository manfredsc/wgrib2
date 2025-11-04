/** @file
 * @brief Routines for reading GRIB files sequentially.
 *
 * There are two ways to read a grib file
 *  1. Random access
 *  2. Sequentially
 * 
 * Example of Random Access (reading in controlled by index)
 * @code{.sh}
 * wgrib2 IN.grb | grep HGT | wgrib2 -i IN.grb -grib OUT.grb
 * @endcode
 * 
 * Example of Sequential Access
 * @code{.sh}
 * wgrib2  IN.grb -if "HGT:" -grib OUT.grb -endif
 * @endcode
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 1/2007 | W. Ebisuzaki | Initial
 * 1/2007 | M. Schwarb   | Cleanup
 * 6/2009 | W. Ebisuzaki | Fix repeated bitmaps
 * 1/2011 | W. Ebisuzaki | Added sequential input
 *
 * @note Reading from pipes and handling files > 2 GB on a 32-bit CPU requires sequential access.
 *
 * @author Public Domain: Wesley Ebisuzaki @date 1/2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "grb2.h"
#include "wgrib2.h"

#ifndef DISABLE_STAT
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

/*
 * rd_grib2_msg_seq_file.c *                              Wesley Ebisuzaki
 *
 * unsigned char rd_grib2_msg(FILE *input, long int pos, *int len)
 *
 * to read grib2 message
 *
 *    msg = rd_grib_msg(input, position, &len)
 *
 *    input is the file
 *    position is the byte location (should be set to zero for first record 
 *    msg = location of message
 *
 *    to get next record: *  position = position + len;
 *
 * rd_grib_msg allocates its own buffer which can be seen by the
 * parsing routines
 *
 * 1/2007 cleanup M. Schwarb
 * 6/2009 fix repeated bitmaps W. Ebisuzuaki
 * 1/2011 added seq input W. Ebisuzaki
 */

/** Buffer allocation size. */
#define BUFF_ALLOC0	(1024*64)


/* ascii values of GRIB */
#define G       71  /**< ASCII value of 'G' in GRIB */
#define R       82  /**< ASCII value of 'R' in GRIB */
#define I       73  /**< ASCII value of 'I' in GRIB */
#define B       66  /**< ASCII value of 'B' in GRIB */

/** Memory Buffers */
extern unsigned char *mem_buffer[N_mem_buffers];

/** Memory buffer sizes. */
extern size_t mem_buffer_size[N_mem_buffers];

/** Memory buffer positions. */
extern size_t mem_buffer_pos[N_mem_buffers];


/*
 * fopen_open_file
 *
 * ffopens a file
 *  sets up structure for reading the file
 *
 */

/**
 * Opens a file and sets up the structure for reading the file.
 *
 * @param file Pointer to the seq_file structure to initialize.
 * @param filename Pointer to the name of the file to open.
 * @param open_mode Mode in which to open the file (e.g., "r" for read).
 *
 * @return 0 on success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
int fopen_file(struct seq_file *file, const char *filename, const char *open_mode) {
#ifndef DISABLE_STAT
    struct stat stat_buf;
#endif
    int i, j;

    file->unget_cnt = 0;
    file->pos = 0;
    file->buffer = NULL;

    i = strlen(filename);
    if (i == 0) fatal_error("fopen_file: missing filename", "");
    j = i < STRING_SIZE ?  i : STRING_SIZE - 1;
    for (i = 0; i < j; i++) file->filename[i] = filename[i];
    file->filename[j] = 0;

    if (strcmp("-",filename) == 0) {
        if (open_mode[0] == 'r') {
            file->cfile = stdin;
            file->file_type = PIPE;
            strncpy(file->filename,"(stdin)",8);
            return 0;
        }
        if (open_mode[0] == 'w') {
            file->cfile = stdout;
            file->file_type = PIPE;
            strncpy(file->filename,"(stdout)",9);
            return 0;
        }
        fatal_error_ss("fopen_file, programming error unexpected open mode (%s) for file %s", open_mode,filename);
    }

    if (strncmp(filename,"@mem:",5) == 0) {
        i = atoi(filename+5);
        if (i < 0 || i >= N_mem_buffers || (i == 0 && filename[5] != '0')) {
            fatal_error_i("fopen_file: @mem:N  N from 0..%d", N_mem_buffers-1);
        }
        file->cfile = NULL;
        file->file_type = MEM;
        file->n_mem_buffer = i;
        // mem_buffer_pos[i] = 0;
        if (open_mode[0] == 'w') {
            free_mem_buffer(i);
            mem_buffer_pos[i] = 0;
        }
        else if (open_mode[0] == 'a') {
            mem_buffer_pos[i] = mem_buffer_size[i];
        }
        return 0;
    }

    file->file_type = NOT_OPEN;
    file->cfile = ffopen(filename,open_mode);
    if (file->cfile == NULL) return 1;
    if (strncmp(filename,"@tmp:",5) == 0) {
        file->file_type = DISK;
        return 0;
    }

#ifdef DISABLE_STAT
    /* treat all named files as disk files */
    file->file_type = DISK;
#else
    if (stat(filename, &stat_buf) != -1) {
	if (S_ISREG(stat_buf.st_mode)) {
            file->file_type = DISK;
        }
        else if (S_ISDIR(stat_buf.st_mode)) {
                fatal_error("grib input is a directory: %s",filename);
        }
        else if (S_ISCHR(stat_buf.st_mode)) {
            file->file_type = PIPE;
        }
        else if (S_ISBLK(stat_buf.st_mode)) {
            file->file_type = PIPE;
        }
        else if (S_ISFIFO(stat_buf.st_mode)) {
            file->file_type = PIPE;
        }
        else {
            fprintf(stderr, "stat() function had unknown type file type, assumed DISK\n");
            file->file_type = DISK;
        }
    }
    else {
        fprintf(stderr, "stat() function did no work, assumed to be a disk file\n");
        file->file_type = DISK;
    }
#endif
    return 0;
}

/**
 * Closes a file and cleans up the associated structure.
 *
 * @param file Pointer to the seq_file structure to clean up.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
void fclose_file(struct seq_file *file) {

    /* do not close stdin or stdout */
    if (file->cfile == stdin || file->cfile == stdout) return;

    if (file->file_type == PIPE || file->file_type == DISK) ffclose(file->cfile);
    if (file->buffer != NULL) {
        if (file->file_type == PIPE || file->file_type == DISK)  {
            free(file->buffer);
            file->buffer = NULL;
            file->buffer_size = 0;
        }
    }
    file->file_type = NOT_OPEN;
    return;
}

/** Buffer size for fseek operations. */
#define FSEEK_BUF_SIZE 4*4096

/**
 * Seeks to a specific position in a file.
 *
 * @param file Pointer to the seq_file structure.
 * @param position The position to seek to.
 * @param whence The reference point for the position (SEEK_SET, SEEK_CUR, SEEK_END).
 * 
 * @return 0 on success, error code otherwise.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
int fseek_file(struct seq_file *file, long position, int whence) {

    char buffer[FSEEK_BUF_SIZE];
    int i;
    size_t iret;

    file->unget_cnt = 0;
    if (file->file_type == DISK) {
        if (fseek(file->cfile, position, whence) == -1) return -1;
        if (whence == SEEK_SET) file->pos = position;
        else file->pos = ftell(file->cfile);
    }
    else if (file->file_type == MEM) {
        if (fseek_mem(file->n_mem_buffer, position, whence) == -1) return -1;
        file->pos = ftell_mem(file->n_mem_buffer);
    }
    else if (file->file_type == PIPE && whence == SEEK_CUR) {
        file->pos += position;
        while (position > 0) {
            i = (position > FSEEK_BUF_SIZE ? FSEEK_BUF_SIZE : position);
            iret = fread_file(&(buffer[0]), (size_t) i, (size_t) 1, file);
            if (iret != 1) fatal_error("fseek_file: pipe read failed","");
            position -= i;
        }    
    }
    else {
        fatal_error("fseek_file: %s cannot seek",file->filename);
    }
    return 0;
}

/**
 * Gets the current position in a file.
 *
 * @param file Pointer to the seq_file structure.
 * 
 * @return The current position in the file, or -1 on error.
 * 
 * @author Wesley Ebisuzaki @date 1/2007
 */
long ftell_file(struct seq_file *file) {
    if (file->file_type == DISK) return ftell(file->cfile);
    if (file->file_type == PIPE) return file->pos;
    if (file->file_type == MEM) return ftell_mem(file->n_mem_buffer);
    return -1;
}

/**
 * Reads a line from a file.
 *
 * @param s Pointer to the buffer to store the line.
 * @param size The maximum size of the buffer.
 * @param file Pointer to the seq_file structure.
 *
 * @return Pointer to the buffer containing the line, or NULL on error.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
char *fgets_file(char *s, int size, struct seq_file *file) {
    if (file->file_type == DISK || file->file_type == PIPE) return fgets(s, size, file->cfile);
    if (file->file_type == MEM) return fgets_mem(s, size, file->n_mem_buffer);
    return NULL;
}

/**
 * Gets the next character from the file. Only this routine handles multiple ungetc.
 *
 * @param file Pointer to the seq_file structure.
 * 
 * @return The next character from the file, or EOF on error.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
static int getchar_seq_file(struct seq_file *file) {
    file->pos++;
    if (file->unget_cnt) { return file->unget_buf[--file->unget_cnt]; }
    return fgetc_file(file);
}

/**
 * Pushes a character back onto the input stream.
 *
 * @param file Pointer to the seq_file structure.
 * @param c The character to push back.
 *
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 1/2007
 */
static int unget_seq_file(struct seq_file *file, int c) {
    file->pos--;
    file->unget_buf[file->unget_cnt++] = (unsigned char) c;
    return 0;
}

/**
 * Reads data from a file.
 *
 * @param ptr Pointer to the buffer to store the data.
 * @param size Size of each element to read.
 * @param nmemb Number of elements to read.
 * @param file Pointer to the seq_file structure.
 *
 * @return The number of elements read, or 0 on error.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
size_t fread_file(void *ptr, size_t size, size_t nmemb, struct seq_file *file) {
    size_t iret;

    if (file->file_type == DISK || file->file_type == PIPE) {
        iret = fread(ptr, size, nmemb, file->cfile);
        if (ferror(file->cfile)) fprintf(stderr,"\nWARNING: file read error for %s\n", file->filename);
        return iret;
    }
    if (file->file_type == MEM) return fread_mem(ptr, size, nmemb, file->n_mem_buffer);
    return 0;
}

/**
 * Gets the next character from the file.
 *
 * @param file Pointer to the seq_file structure.
 *
 * @return The next character from the file, or EOF on error.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
int fgetc_file(struct seq_file *file) {
    int n;
    if (file->file_type == DISK || file->file_type == PIPE) return fgetc(file->cfile);
    if (file->file_type == MEM) {
        n = file->n_mem_buffer;
        if (mem_buffer_size[n] > mem_buffer_pos[n]) {
            return mem_buffer[n][mem_buffer_pos[n]++];
        }
    }
    return EOF;
}

/**
 * Writes data to a file.
 *
 * @param ptr Pointer to the data to write.
 * @param size Size of each element to write.
 * @param nmemb Number of elements to write.
 * @param file Pointer to the seq_file structure.
 *
 * @return The number of elements written, or 0 on error.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
size_t fwrite_file(const void *ptr, size_t size, size_t nmemb, struct seq_file *file) {
    size_t iret;

    if (file->file_type == DISK || file->file_type == PIPE) {
        iret = fwrite(ptr, size, nmemb, file->cfile);
        if (feof(file->cfile)) fatal_error("write error (EOF) on %s",file->filename);
        if (ferror(file->cfile)) fatal_error("write error on %s",file->filename);
        return iret;
    }
    if (file->file_type == MEM) return fwrite_mem(ptr, size, nmemb, file->n_mem_buffer);
    return (size_t) 0;
}

/**
 * Flushes the output buffer of a file.
 * 
 * @param file Pointer to the seq_file structure.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
void fflush_file(struct seq_file *file) {
    if (file->file_type == DISK || file->file_type == PIPE) fflush(file->cfile);
    return;
}

/**
 * Reads a GRIB2 message from a sequential file.
 *
 * @param sec Array of pointers to the sections of the GRIB2 message.
 * @param input Pointer to the seq_file structure.
 * @param pos Pointer to the position in the file.
 * @param len Pointer to the length of the message.
 * @param num_submsgs Pointer to the number of submessages.
 *
 * @return Pointer to the GRIB2 message, or NULL on error.
 *
 * @author Wesley Ebisuzaki @date 1/2007
 */
unsigned char *rd_grib2_msg_seq_file(unsigned char **sec, struct seq_file *input, long int *pos, 
        unsigned long int *len, int *num_submsgs) {
    int i, c, c1, c2, c3, c4;
    size_t j, len_grib;
    unsigned char *p, *end_of_msg;

    /* setup grib buffer */
    if (input->buffer == NULL) {
        if ((input->buffer = (unsigned char *) malloc(BUFF_ALLOC0)) == NULL) {
            fatal_error("rd_grib2_msg_seq: memory allocation for %s",input->filename);
        }
        input->buffer_size = BUFF_ALLOC0;
    }

    /* search for GRIB...2 */

    while (1) {
        c = getchar_seq_file(input);
        if (c == EOF) { *len = 0; return NULL; }
        if (c != G) continue;
        if ( (c = getchar_seq_file(input)) != R) { unget_seq_file(input, c); continue; }
        if ( (c = getchar_seq_file(input)) != I) { unget_seq_file(input, c); continue; }
        if ( (c = getchar_seq_file(input)) != B) { unget_seq_file(input, c); continue; }
        c1 = getchar_seq_file(input);
        c2 = getchar_seq_file(input);
        c3 = getchar_seq_file(input);
        c4 = getchar_seq_file(input);
        if (c4 == 1) {
            fprintf(stderr,"rd_grib2_seq_msg, grib1 message ignored for %s, use wgrib\n",input->filename);
            continue;
        }
        if (c4 != 2) {
            unget_seq_file(input, c4);
            unget_seq_file(input, c3);
            unget_seq_file(input, c2);
            unget_seq_file(input, c1);
            continue;
        }
        input->buffer[0] = G;
        input->buffer[1] = R;
        input->buffer[2] = I;
        input->buffer[3] = B;
        input->buffer[4] = c1;
        input->buffer[5] = c2;
        input->buffer[6] = c3;
        input->buffer[7] = c4;

        /* fill in the size 8-15, unget buffer is empty */

        for (i = 0; i < 8; i++) { 
            input->buffer[8+i] = c = getchar_seq_file(input);
            if (c == EOF) {
                *len = 0;
                return NULL;
            }
        }
        break;
    }

    *len = len_grib = uint8(input->buffer+8);

    *pos = input->pos - 16;

    if (input->buffer_size < len_grib) {
        input->buffer_size = (len_grib*5)/4;
        input->buffer = (unsigned char *) realloc((void *) input->buffer, input->buffer_size);
        if (input->buffer == NULL) fatal_error("rd_grib2_msg_seq_file for %s: grib message size %lu too large", 
            input->filename,input->buffer_size);
    }

    j=fread_file(input->buffer+16, sizeof (unsigned char), len_grib-16, input);
    input->pos += j;
    
    if (j != len_grib-16) fatal_error("rd_grib2_msg_seq_file for %s, read outside of file, bad grib file",input->filename);

    sec[0] = input->buffer;
    sec[8] = sec[0] + len_grib - 4;
    if (sec[8][0] != 55 || sec[8][1] != 55 || sec[8][2] != 55 || sec[8][3] != 55) {
        fatal_error("rd_grib2_msg_seq_file for %s, missing end section ('7777')",input->filename);
    }

    /* scan message for number of submessages and perhaps for errors */
    p = sec[0] +  GB2_Sec0_size;
    end_of_msg = sec[0] + len_grib;

    i = 0;
    while (p < sec[8]) {
        if (p[4] == 7) i++;
        if (uint4(p) < 5) fatal_error_i("rd_grib2_msg_file: illegal grib %s, section length, section %i", input->filename, p[4]);
        p += uint4(p);
        if (p > end_of_msg) fatal_error("rd_grib2_msg_file: bad grib format for %s",input->filename);
    }
    if (p != sec[8]) {
        fatal_error("rd_grib2_msg: illegal format, end section expected for %s",input->filename);
    }
    *num_submsgs = i;

    *len = len_grib;
    return sec[0];
}
