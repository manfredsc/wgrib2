/** @file
 * @brief This file contains utility functions for copying and freeing GRIB2 sections and data.
 * @author Public Domain: Wesley Ebisuzaki @date 8/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "grb2.h"
#include "wgrib2.h"

/**
 * Copies the GRIB2 sections and data from one pointer to another.
 * 
 * @param sec Pointer to the original GRIB2 sections.
 * @param clone_sec Pointer to the destination for the copied GRIB2 sections.
 * 
 * @return 0 on success, or an error code if memory allocation fails.
 * 
 * @author Wesley Ebisuzaki @date 8/2008
 */
int copy_sec(unsigned char **sec, unsigned char **clone_sec) {

    int i;
    unsigned int size[9];

//  get size of each secion

    size[0] = GB2_Sec0_size;
    size[1] = GB2_Sec1_size(sec);
    size[2] = 0;
    if (sec[2]) size[2] = GB2_Sec2_size(sec);
    size[3] = GB2_Sec3_size(sec);
    size[4] = GB2_Sec4_size(sec);
    size[5] = GB2_Sec5_size(sec);
    size[6] = GB2_Sec6_size(sec);
    size[7] = GB2_Sec7_size(sec);
    size[8] = GB2_Sec8_size;

//  allocate and copy memory

    for (i = 0; i < 9; i++) {
        if (size[i] > 0) {
            if ((clone_sec[i] = (unsigned char *) malloc(size[i])) == NULL) 
                fatal_error_i("memory allocation failed copy_sec %d",i);
            memcpy(clone_sec[i], sec[i], size[i]);
        }
        else {
            clone_sec[i] = NULL;
        }
    }	

    return 0;
}

/**
 * Frees the memory allocated for the GRIB2 sections.
 * 
 * @param clone_sec Pointer to the GRIB2 sections to be freed.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 8/2008
 */
int free_sec(unsigned char **clone_sec) {
    int i;
    for (i = 0; i < 9; i++) {
        if (clone_sec[i]) free(clone_sec[i]);
        clone_sec[i] = NULL;
    }
    return 0;
}

/**
 * Initializes the GRIB2 sections to NULL pointers.
 * 
 * @param clone_sec Pointer to the GRIB2 sections to be initialized.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 8/2008
 */
int init_sec(unsigned char **clone_sec) {
    int i;
    for (i = 0; i < 9; i++) {
        clone_sec[i] = NULL;
    }
    return 0;
}

/**
 * Copies the data from one float array to another.
 * 
 * @param data Pointer to the original float array.
 * @param ndata Number of elements in the original float array.
 * @param clone_data Pointer to the destination for the copied float array.
 * 
 * @return 0 on success, or an error code if memory allocation fails.
 * 
 * @author Wesley Ebisuzaki @date 8/2008
 */
int copy_data(float *data, unsigned int ndata, float **clone_data) {

    float *fp;
    unsigned int i;

    *clone_data = fp = (float *) malloc(((size_t) ndata) * sizeof(float));
    if (fp == NULL) fatal_error("memory allocation clone_data","");

    for (i = 0; i < ndata; i++) {
        fp[i] = data[i];
    }
    return 0;
}

/**
 * Frees the memory allocated for the float array.
 * 
 * @param clone_data Pointer to the float array to be freed.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 8/2008
 */
int free_data(float *clone_data) {
    free(clone_data);
    return 0;
}
