/** @file
 * @brief This file contains the implementation of the -ave0 option and some helper functions.
 * The -ave0 option is the old (v2.0.6) version of the -ave option.
 * @author Public Domain: Wesley Ebisuzaki @date 04/2009
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 04/2009 | W. Ebisuzaki | Initial
 * 04/2010 | W. Ebisuzaki | add means of means
 * 04/2013 | W. Ebisuzaki | added pdt 4.11 (ensemble)
 * 12/2014 | W. Ebisuzaki | set use_scale to zero, optimizations
 * 01/2015 | W. Ebisuzaki | removed set use_scale
 * 03/2016 | W. Ebisuzaki | added pdt 2 and 12
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag  */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Number of grid points in the x direction. */
extern int nx;

/** Number of grid points in the y direction. */
extern int ny;

/** Flag to indicate whether to save translation information. */
extern int save_translation;

/** Flush of output flag. */
extern int flush_mode;

/** Pointer to the translation array. */
extern unsigned int *translation;

/** Use scaling flag. */
extern int use_scale;

/** Decoding scale flag. */
extern int dec_scale;

/** Binary scale flag. */
extern int bin_scale;

/** Number of bits wanted. */
extern int wanted_bits;

/** Maximum number of bits. */
extern int max_bits;

/** Output GRIB type. */
extern enum output_grib_type grib_type;

/** Structure to hold average calculation data. */
struct ave_struct {
    double *sum; /**< Sum of values. */
    int *n; /**< Count of values. */
    unsigned int n_sum; /**< Total number of values. */
    int has_val; /**< Flag to indicate if values are present. */
    int n_fields; /**< Number of fields. */
    int n_missing; /**< Number of missing values. */
    int dt; /**< Time interval. */
    int dt_unit; /**< Time unit. */
    int nx; /**< Number of grid points in the x direction. */
    int ny; /**< Number of grid points in the y direction. */
    unsigned char *first_sec[9]; /**< First section data. */
    unsigned char *next_sec[9]; /**< Next section data. */
    int use_scale; /**< Flag to indicate whether to use scaling. */
    int dec_scale; /**< Flag to indicate the decoding scale. */
    int bin_scale; /**< Flag to indicate the binary scale. */
    int wanted_bits; /**< Flag to indicate the number of bits wanted. */
    int max_bits; /**< Flag to indicate the maximum number of bits. */
    enum output_grib_type grib_type; /**< Flag to indicate the output GRIB type. */
    int year0; /**< Year of the lowest reference time. */
    int month0; /**< Month of the lowest reference time. */
    int day0; /**< Day of the lowest reference time. */
    int hour0; /**< Hour of the lowest reference time. */
    int minute0; /**< Minute of the lowest reference time. */
    int second0; /**< Second of the lowest reference time. */
    int year1; /**< Year of the current reference time. */
    int month1; /**< Month of the current reference time. */
    int day1; /**< Day of the current reference time. */
    int hour1; /**< Hour of the current reference time. */
    int minute1; /**< Minute of the current reference time. */
    int second1; /**< Second of the current reference time. */
    int year2; /**< Year of the verification time. */
    int month2; /**< Month of the verification time. */
    int day2; /**< Day of the verification time. */
    int hour2; /**< Hour of the verification time. */
    int minute2; /**< Minute of the verification time. */
    int second2; /**< Second of the verification time. */
    struct seq_file out; /**< Output file structure. */
};

static int do_ave(struct ave_struct *save);
static int free_ave_struct(struct ave_struct *save);
static int init_ave_struct(struct ave_struct *save, unsigned int ndata);
static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing);

/**
 * Frees the memory allocated for the ave_struct.
 * 
 * @param save Pointer to the ave_struct to be freed.
 * 
 * @return 0 on success, does not return error codes.
 * 
 * @author Wesley Ebisuzaki @date 04/2009
 */
static int free_ave_struct(struct ave_struct *save) {
    if (save->has_val == 1) {
        free(save->sum);
        free(save->n);
        free_sec(save->first_sec);
        free_sec(save->next_sec);
    }
    free(save);
    return 0;
}

/**
 * Initializes the ave_struct with the given number of data points.
 * 
 * @param save Pointer to the ave_struct.
 * @param ndata Number of data points.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 04/2009
 */
static int init_ave_struct(struct ave_struct *save, unsigned int ndata) {
    unsigned int i;
    if (save->has_val == 0 || save->n_sum != ndata) {
        if (save->has_val == 1) {
            free(save->sum);
            free(save->n);
        }
        if ((save->sum = (double *) malloc( ((size_t) ndata) * sizeof(double))) == NULL)
        fatal_error("ave: memory allocation problem: val","");
        if ((save->n = (int *) malloc(((size_t) ndata) * sizeof(int))) == NULL)
        fatal_error("ave: memory allocation problem: val","");
    }

    for (i=0; i < ndata; i++) {
        save->n[i] = 0;
        save->sum[i] = 0.0;
    }
    save->n_sum = ndata;
    save->has_val = 1;
    save->n_fields = 0;
    save->n_missing = 0;
    free_sec(save->first_sec);
    free_sec(save->next_sec);
    return 0;
}

/**
 * Adds data to the ave_struct.
 * 
 * @param save Pointer to the ave_struct.
 * @param sec Pointer to the section data.
 * @param data Pointer to the data array.
 * @param ndata Number of data points.
 * @param missing Number of missing data points.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 04/2009
 */
static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata, int missing) {
    unsigned int i;

    if (save->n_sum != ndata) fatal_error("add_to_ave: dimension mismatch","");

    /* the data needs to be translated from we:sn to raw, need to 
       do it now, translation[] may be different if called from finalized phase */

    if (translation == NULL) {
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->sum[i])) {
                save->sum[i] += data[i];
                save->n[i]++;
            }
        }
    }
    else {
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->sum[i])) {
                save->sum[translation[i]] += data[i];
                save->n[translation[i]]++;
            }
        }
    }
    save->n_fields += 1;
    if (save->n_fields == 1) {
        save->nx = nx;
        save->ny = ny;
        save->use_scale = use_scale;
        save->dec_scale = dec_scale;
        save->bin_scale = bin_scale;
        save->wanted_bits = wanted_bits;
        save->max_bits = max_bits;
        save->grib_type = grib_type;
    }
    save->n_missing += missing;

    // update current reference time
    get_time(sec[1]+12, &save->year1, &save->month1, &save->day1, &save->hour1, &save->minute1, &save->second1);

    // update current verf time
    if (verftime(sec, &save->year2, &save->month2, &save->day2, &save->hour2, &save->minute2, &save->second2) != 0) {
	    fatal_error("add_to_ave_struct: could not find verification time","");
    }
    return 0;
}


/**
 * Performs the average calculation based on the data stored in the ave_struct.
 * 
 * @param save Pointer to the ave_struct containing the data.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 04/2009
 */
static int do_ave(struct ave_struct *save) {
    int j, n, pdt;
    unsigned int i, ndata;
    float *data;
    unsigned char *p, *sec4;
    double factor;

    sec4 = NULL;
    if (save->has_val == 0) return 0; 

    ndata = save->n_sum;
    if ((data = (float *) malloc(sizeof(float) * ((size_t) ndata))) == NULL) fatal_error("ave: memory allocation","");
    factor = 1.0 / save->n_fields;
    for (i = 0; i < ndata; i++) {
        if (save->n[i] != save->n_fields) data[i] = UNDEFINED;
        else data[i] = factor * save->sum[i];
    }

    pdt = GB2_ProdDefTemplateNo(save->first_sec);

    // average of an analysis or forecast

    if (pdt == 0) {
        sec4 = (unsigned char *) malloc(58 * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("ave: memory allocation","");
        for (i = 0; i < 34; i++) {
            sec4[i] = save->first_sec[4][i];
        }
        uint_char((unsigned int) 58, sec4);		// length
        sec4[8] = 8;			// pdt
        // verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+34);
        sec4[41] = 1;
        uint_char(save->n_missing, sec4+42);
        sec4[46] = 0;			// average
        sec4[47] = 1;			// rt++
        sec4[48] = save->dt_unit;					// total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+49);
        sec4[53] = save->dt_unit;					// time step
        uint_char(save->dt, sec4+54);
    }

    // average of an ensemble forecast, use pdt 4.11

    else if (pdt == 1) {
        sec4 = (unsigned char *) malloc(61 * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("fcst_ave: memory allocation","");
        for (i = 0; i < 37; i++) {
            sec4[i] = save->first_sec[4][i];
        }
        uint_char((unsigned int) 61, sec4);             // length
        sec4[8] = 11;                    		// pdt
        // verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+37);
        sec4[44] = 1;                                   // 1 time range
        uint_char(save->n_missing, sec4+45);
        sec4[49] = 0;                                   // average
        sec4[50] = 1;                                   // rt=constant
        sec4[51] = save->dt_unit;                                       // total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+52);
        sec4[56] = save->dt_unit;                                       // time step
        uint_char(save->dt, sec4+57);
    }

    // average of derived fcst base on all ens members, use pdt 4.12

    else if (pdt == 2) {
        sec4 = (unsigned char *) malloc(60 * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("fcst_ave: memory allocation","");
        for (i = 0; i < 36; i++) {
            sec4[i] = save->first_sec[4][i];
        }
        uint_char((unsigned int) 60, sec4);             // length
        sec4[8] = 12;                    		// pdt
        // verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+36);
        sec4[43] = 1;                                   // 1 time range
        uint_char(save->n_missing, sec4+44);
        sec4[48] = 0;                                   // average
        sec4[49] = 1;                                   // rt=constant
        sec4[50] = save->dt_unit;                       // total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+51);
        sec4[55] = save->dt_unit;                       // time step
        uint_char(save->dt, sec4+56);
    }

    // average of an average or accumulation

    else if (pdt == 8) {
        i = GB2_Sec4_size(save->first_sec);
        n = save->first_sec[4][41];
        if (i != 46 + 12*n) fatal_error("ave: invalid sec4 size for pdt=8","");

        // keep pdt == 8 but make it 12 bytes bigger
        sec4 = (unsigned char *) malloc( (i+12) * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("ave: memory allocation","");

        uint_char((unsigned int) i+12, sec4);		// new length

        for (i = 4; i < 34; i++) {			// keep base of pdt
            sec4[i] = save->first_sec[4][i];
        }
	
	    // new verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+34);

        // number of stat-proc loops is increased by 1
        sec4[41] = n + 1;

        // copy old stat-proc loops 
        // for (j = n*12-1;  j >= 0; j--) sec4[58+j] = save->first_sec[4][46+j];
        for (j = 0; j < n*12; j++) sec4[46+12+j] = save->first_sec[4][46+j];

        uint_char(save->n_missing, sec4+42);
        sec4[46] = 0;			// average
        sec4[47] = 1;			// rt++
        sec4[48] = save->dt_unit;						// total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+49);	// processing number
        sec4[53] = save->dt_unit;						// time step
        uint_char(save->dt, sec4+54);
    }

    // pdt 4.12 -> pdt 4.12 ave -> ave of ave

    else if (pdt == 12) {
        i = GB2_Sec4_size(save->first_sec);
        n = save->first_sec[4][43];			// number of time ranges
        if (i != 48 + 12*n) fatal_error("ave: invalid sec4 size for pdt=12","");

        // keep pdt == 12 but make it 12 bytes bigger
        sec4 = (unsigned char *) malloc( (i+12) * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("ave: memory allocation","");

        uint_char((unsigned int) i+12, sec4);           // new length

        for (i = 4; i < 36; i++) {                      // keep base of pdt
            sec4[i] = save->first_sec[4][i];
        }

        // new verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+36);

        // number of stat-proc loops is increased by 1
        sec4[43] = n + 1;

        // copy old stat-proc loops
        for (j = 0; j < n*12; j++) sec4[48+12+j] = save->first_sec[4][48+j];

        uint_char(save->n_missing, sec4+44);
        sec4[48] = 0;                   // average
        sec4[49] = 1;                   // rt++
        sec4[50] = save->dt_unit;                                               // total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+51);        // processing number
        sec4[55] = save->dt_unit;                                               // time step
        uint_char(save->dt, sec4+56);
    }


    else {
	    fatal_error_i("ave with pdt %d is not supported",pdt);
    }


    // write grib file
    p = save->first_sec[4];
    save->first_sec[4] = sec4;

    grib_wrt(save->first_sec, data, ndata, save->nx, save->ny, 
	save->use_scale, save->dec_scale, save->bin_scale, 
	save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    if (flush_mode) fflush_file(&(save->out));
    save->first_sec[4] = p;
    free(data);
    free(sec4);
    return 0;
}

/*
 * HEADER:000:ave0:output:2:average X=time step, Y=output grib file needs file is special order
 */

/**
 * Calculates temporal averages of grib data and writes the results to a specified output file.
 * This is the old version (v2.0.6) of the -ave option.
 * 
 * ## Usage
 * -ave0 (time interval) (output grib file)
 * 
 * The time interval is the delta time for averaging (e.g., "6hr", "1dy").
 * 
 * @param ARG2 ???
 * 
 * @return 0 for success, error code otherwise.
 * 
 * ## Example: 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 04/2009
 */
int f_ave0(ARG2) {

    struct ave_struct *save;

    int i, pdt, new_type;
    int year, month, day, hour, minute, second;
    int tyear, tmonth, tday, thour, tminute, tsecond;
    int missing;
    char string[10];

    // initialization

    if (mode == -1) {
        save_translation = decode = 1;

	    // allocate static structure

        *local = save = (struct ave_struct *) malloc( sizeof(struct ave_struct));
        if (save == NULL) fatal_error("memory allocation f_ave","");

        i = sscanf(arg1, "%d%2s", &save->dt,string);
        if (i != 2) fatal_error("ave: delta-time: (int)(2 characters) %s", arg1);
        save->dt_unit = -1;
        if (strcmp(string,"hr") == 0) save->dt_unit = 1;
        else if (strcmp(string,"dy") == 0) save->dt_unit = 2;
        else if (strcmp(string,"mo") == 0) save->dt_unit = 3;
        else if (strcmp(string,"yr") == 0) save->dt_unit = 4;
	    if (save->dt_unit == -1) fatal_error("ave: unsupported time unit %s", string);

        if (fopen_file(&(save->out), arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }
        save->has_val = 0;
        save->n = NULL;
        save->sum = NULL;
        init_sec(save->first_sec);
        init_sec(save->next_sec);

        return 0;
    }

    save = (struct ave_struct *) *local;

    if (mode == -2) {			// cleanup
        if (save->has_val == 1) {
            do_ave(save);
        }
        fclose_file(&(save->out));
        free_ave_struct(save);
        return 0;
    }

    // if data

    if (mode >= 0) {
        // 1/2015 use_scale = 0;
        pdt = GB2_ProdDefTemplateNo(sec);

        if (mode == 98) fprintf(stderr,"ave: pdt=%d\n",pdt);
        // only support pdt == 0, 1, 2, 8, 12
        // if (pdt != 0 && pdt != 1 && pdt != 8) return 0;
        if (pdt != 0 && pdt != 1 && pdt != 2 && pdt != 8 && pdt != 12) return 0;

        // first time through .. save data and return

        if (save->has_val == 0) {		// new data: write and save
            init_ave_struct(save,ndata);
            add_to_ave_struct(save, sec, data, ndata, 0);

            // copy sec
            copy_sec(sec, save->first_sec);
            copy_sec(sec, save->next_sec);

            // time0 = lowest reference time
            // time1 = current reference time
            // time2 = verification time

            get_time(sec[1]+12,&save->year0, &save->month0, &save->day0, &save->hour0, &save->minute0, &save->second0);

            save->year1 = save->year0;
            save->month1 = save->month0;
            save->day1 = save->day0;
            save->hour1 = save->hour0;
            save->minute1 = save->minute0;
            save->second1 = save->second0;

            if (verftime(sec, &save->year2, &save->month2, &save->day2, &save->hour2, &save->minute2, &save->second2) != 0) {
                fatal_error("ave: could not determine the verification time","");
            }

            save->has_val = 1;
            return 0;
        }

        // check to see if new variable

        new_type = 0;

        // get the reference time of field
        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);

        // get the reference time of last field
        tyear = save->year1;
        tmonth = save->month1;
        tday = save->day1;
        thour = save->hour1;
        tminute = save->minute1;
        tsecond = save->second1;

        // make (tyear,tmonth...) the expected date
        add_time(&tyear, &tmonth, &tday, &thour, &tminute, &tsecond, save->dt, save->dt_unit);

        missing = 0;
        while ((i=cmp_time(year,month,day,hour,minute,second,tyear,tmonth,tday,thour,tminute,tsecond)) > 0) {
            missing++;
            add_time(&tyear, &tmonth, &tday, &thour, &tminute, &tsecond, save->dt, save->dt_unit);
        }

        if (i != 0) new_type = 1;
        if (mode == 98) fprintf(stderr, "ave: compare ref time new_type = %d missing=%d\n", new_type, missing);

        if (new_type == 0) {
            if (same_sec0(sec,save->first_sec) == 0 ||
                    same_sec1_not_time(mode,sec,save->first_sec) == 0 ||
                    same_sec3(sec,save->first_sec) == 0 ||
                    same_sec4_not_time(mode, sec,save->first_sec) == 0) 
                new_type = 1;
            if (mode == 98) fprintf(stderr, "ave: testsec save_sec0=%d same_sec1_not_time=%d same_sec3=%d same_sec4_not_time=%d\n", 
            same_sec0(sec,save->first_sec),
            same_sec1_not_time(0,sec,save->first_sec),
            same_sec3(sec,save->first_sec),
            same_sec4_not_time(0,sec,save->first_sec));
        }
        if (mode == 98) fprintf(stderr, "ave: new_type %d\n", new_type);

        // if data is the same as the previous, update the sum

        if (new_type == 0) {		// update sum
            if (mode == 98) fprintf(stderr, "ave: update sum\n");
            add_to_ave_struct(save, sec, data, ndata, missing);
            return 0;
        }

        // new field, do grib output and save current data

        do_ave(save);
        init_ave_struct(save, ndata);
        add_to_ave_struct(save, sec, data, ndata, 0);
        copy_sec(sec, save->first_sec);
        copy_sec(sec, save->next_sec);
        return 0;
    }
    return 0;
}
