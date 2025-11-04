/** @file
 * @brief This program takes the input of N files or pipes containing grib2 files
 * and merges them into one file in a round-robin order.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 05/2009 | W. Ebisuzaki | Initial
 * 08/2012 | M. Schwarb | added declarations, exit(0)
 * 02/2015 | W. Ebisuzaki | can write to stdout by filename='-'
 * 01/2018 | W. Ebisuzaki | increase N to 200, print N in description
 * 05/2018 | W. Ebisuzaki | increase buffer size, call feof
 * 12/2022 | W. Ebisuzaki | better error messages, list of input files can be 1 file
 * 01/2023 | W. Ebisuzaki | updated for 2023, cmake compile added
 * 05/2025 | W. Ebisuzaki | increase N again (32..200..system limit)
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 05/2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Current Version of gmerge */
#define VERSION "gmerge v1.6 5/2025"

unsigned long int uint8(unsigned char *);
int rd_msg(FILE *, FILE *);

/**
 * Combines two or more streams of grib files. The output takes one grib message from
 * the first file/pipe, then one from the second file/pipe, and so on until you reach
 * the last file/pipe.
 *
 * The program is to split the workload over several copies of wgrib2.
 * 
 * ## Example
 * 
 * @code{.sh}
 * mkfifo pipe1 pipe2
 * wgrib2 IN.grb -for 1::2 -set_grib_type j -grib_out pipe1 & 
 * wgrib2 IN.grb -for 2::2 -set_grib_type j -grib_out pipe2 & 
 * gmerge OUT.grb pipe1 pipe2
 * rm pipe1 pipe2
 * @endcode
 * 
 * OUT.grb contains the jpeg compressed version of IN.grb. The compression was run as 
 * two tasks.
 * 
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * 
 * @return 0 on success, non-zero on error.
 *
 * @author Wesley Ebisuzaki @date 05/2009
 */
int main(int argc, char **argv) {

    FILE *out, *p[argc];
    int eofs[argc];
    int i, n, done;
    /* over allocate p[] and eofs[] by two but can't let size be zero in case of no args  */

    if (argc < 3) {
        fprintf(stderr,"%s combines grib files in round-robin fashion\n", VERSION);
        fprintf(stderr,"%s (output) (list of input grib files)\n", argv[0]);
        exit(8);
    }

    /* open output file */
    if (strcmp(argv[1], "-") == 0) {
        out = stdout;
    }
    else {
        if ((out = fopen(argv[1],"wb")) == NULL) {
            fprintf(stderr,"bad arg: output=%s\n",argv[1]);
            exit(8);
        }
    }

    /* open list of input files */
    n = argc - 2;
    for (i = 0; i < n; i++) {
        p[i] = fopen(argv[i+2], "rb");
        if (p[i] == NULL) {
            fprintf(stderr,"bad file: %s\n",argv[i+2]);
            exit(8);
        }
        eofs[i] = 0;
    }

    /* read in round-robin order */
    done = 0;
    while (done != n) {
        done  = 0;
        for (i = 0; i < n; i++) {
            if (eofs[i] == 1) {
                done++;
            }
            else {
                if (rd_msg(p[i], out)) {
                    done++;
                    eofs[i] = 1;
                }
            }
        }
    }
    exit(0);
}

/** Maximum Buffer Size */
#define BSIZE 4096*8

/**
 * Reads a GRIB2 message from the input file and writes it to the output file.
 *
 * @param in Pointer to the input file.
 * @param out Pointer to the output file.
 *
 * @return 0 on success, non-zero on error.
 *
 * @author Wesley Ebisuzaki @date 05/2009
 */
int rd_msg(FILE *in, FILE *out) {
    long unsigned int n;
    int i,j,k;
    unsigned char header[BSIZE];

    if (feof(in)) return -1;

    i = fread(header, 1, 16, in);
    if (i != 16) return -1;
    if (header[0] != 'G' || header[1] != 'R' || header[2] != 'I' || 
        header[3] != 'B') return -1;

    n = uint8(&(header[8]));

    j = n < BSIZE ? n : BSIZE;
    k = fread(header+16,1,j-16,in);
    if (k != j-16) return -1;

    fwrite(header,1,j,out);
    n -= j;

    while (n) {
        j = n < BSIZE ? n : BSIZE;
        k = fread(header,1,j,in);
        fwrite(header,1,j,out);
        n -= j;
    }
    return 0;
}
