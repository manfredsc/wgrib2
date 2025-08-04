/** @file
 * @brief Utility functions for manipulating GRIB time codes.
 * @author Public Domain: Wesley Ebisuzaki @date 01/2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "CodeTable4_4.h"

/** February 29th day of the year */
#define  FEB29   (31+29)

/** Array of cumulative days at the start of each month (non-leap year). */
static int monthjday[13] = {
    0,31,59,90,120,151,181,212,243,273,304,334,365
};

/**
 * Determines if a year is a leap year.
 * 
 * @param year Year to check.
 * 
 * @return 1 if leap year, 0 otherwise.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
static int leap(int year) {
    if (year % 4 != 0) return 0;
    if (year % 100 != 0) return 1;
    return (year % 400 == 0);
}

/**
 * Checks if a date code is valid.
 * 
 * @param year Year.
 * @param month Month (1-12).
 * @param day Day of month.
 * 
 * @return
 * - 0 Date code is valid.
 * - 1 Invalid year.
 * - 2 Invalid month.
 * - 3 Invalid day.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int check_datecode(int year, int month, int day) {
    int days_in_month;

    if (year < 0) return 1;
    if (month <= 0 || month > 12) return 2;
    if (day < 1) return 3;
    days_in_month = monthjday[month] - monthjday[month-1];
    if (month == 2 && leap(year)) {
        days_in_month++;
    }
    if (day <= days_in_month) return 0;
    return 3;
}

/**
 * Checks if a time code is valid.
 * 
 * @param year Year.
 * @param month Month (1-12).
 * @param day Day of month.
 * @param hour Hour (0-23).
 * @param minute Minute (0-59).
 * @param second Second (0-59).
 * 
 * @return
 * - 0 Time code is valid.
 * - 1 Invalid year.
 * - 2 Invalid month.
 * - 3 Invalid day.
 * - 4 Invalid hour.
 * - 5 Invalid minute.
 * - 6 Invalid second.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */

int check_time(int year, int month, int day, int hour, int minute, int second) {

    if (hour < 0 || hour > 23) return 4;
    if (minute < 0 || minute > 59) return 5;
    if (second < 0 || second > 59) return 6;
    return check_datecode(year, month, day);
}

/**
 * Adds or subtracts a time interval to a time code.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 01/2006 | W. Ebisuzaki | Initial
 * 01/2006 | W. Ebisuzaki | removed floor(), case where floor(24/24.0) == 0
 *                          some day should allow +/- dtime (not needed for this application
 *                          when adding 1 month, make sure day of month is legal)
 * 01/2007 | M. Schwarb, W. Ebisuzaki | Cleanup
 * 04/2015 | M. Schwarb, W. Ebisuzaki | dtime is now integer
 * 
 * @param year Pointer to year.
 * @param month Pointer to month.
 * @param day Pointer to day.
 * @param hour Pointer to hour.
 * @param minute Pointer to minute.
 * @param second Pointer to second.
 * @param dtime Time interval.
 * @param unit Time unit (YEAR, MONTH, DAY, etc).
 * 
 * @return
 * - 0 Success.
 * - 1 Invalid time unit.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int add_time(int *year, int *month, int *day, int *hour, int *minute, int *second, int dtime, int unit) {
    if (dtime == 0) return 0;
    if (dtime > 0) return add_dt(year, month, day, hour, minute, second, dtime, unit);
    return sub_dt(year, month, day, hour, minute, second, -dtime, unit);
}

/**
 * Adds or subtracts a time interval to a full_date struct.
 * 
 * @param date Pointer to full_date struct.
 * @param dtime Time interval.
 * @param unit Time unit.
 * 
 * @return
 * - 0 Success.
 * - 1 Invalid time unit.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int Add_time(struct full_date *date, int dtime, int unit) {
    if (dtime == 0) return 0;
    if (dtime > 0) 
        return add_dt(&(date->year), &(date->month), &(date->day), &(date->hour), &(date->minute), &(date->second), dtime, unit);
    return sub_dt(&(date->year), &(date->month), &(date->day), &(date->hour), &(date->minute), &(date->second), -dtime, unit);
}

/**
 * Adds a time interval to a time code.
 * 
 * @param year Pointer to year.
 * @param month Pointer to month.
 * @param day Pointer to day.
 * @param hour Pointer to hour.
 * @param minute Pointer to minute.
 * @param second Pointer to second.
 * @param dtime Time interval.
 * @param unit Time unit.
 * 
 * @return
 * - 0 Success.
 * - 1 Invalid time unit.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int add_dt(int *year, int *month, int *day, int *hour, int *minute, int *second, int dtime, int unit) {

    int y, m, d, h, mm, s, jday, i, days_in_month;

    y = *year;
    m = *month;
    d = *day;
    h = *hour;
    mm = *minute;
    s = *second;

    if (unit == 255 || unit == -1) return 0;	// no valid time unit
    if (dtime == 0xffffffff) return 0;		// missing dtime

    if (unit == YEAR) {
        *year = y + dtime;
        return 0;
    }
    if (unit == DECADE) {
        *year =  y + (10 * dtime);
        return 0;
    }
    if (unit == CENTURY) {
        *year = y + (100 * dtime);
        return 0;
    }
    if (unit == NORMAL) {
        *year = y + (30 * dtime);
        return 0;
    }
    if (unit == MONTH) {
/*
        if (dtime < 0) {
           i = (-dtime) / 12 + 1;
           y -= i;
           dtime += (i * 12);
        }
*/
        dtime += (m - 1);
        *year = y = y + (dtime / 12);
        *month = m = 1 + (dtime % 12);

        /* check if date code if valid */
        days_in_month = monthjday[m] - monthjday[m-1];
        if (m == 2 && leap(y)) {
            days_in_month++;
        }
        if (days_in_month < d) *day = days_in_month;

        return 0;
    }
    if (unit == SECOND) {
        s += dtime;
        // dtime = floor(s/60.0);
        dtime =  s / 60;
        *second = s - dtime*60;
        if (dtime == 0) return 0;
        unit = MINUTE;
    }
    if (unit == MINUTE) {
        mm += dtime;
        // dtime = floor(mm/60.0);
        dtime = mm /60;
        *minute = mm - dtime*60;
        if (dtime == 0) return 0;
        unit = HOUR;
    }
    if (unit == HOUR3) {
        dtime *= 3;
        unit = HOUR;
    }
    if (unit == HOUR6) {
        dtime *= 6;
        unit = HOUR;
    }
    if (unit == HOUR12) {
        dtime *= 12;
        unit = HOUR;
    }
    if (unit == HOUR) {
        h += dtime;
        // dtime = floor(h/24.0);
        dtime = h / 24;
        *hour = h - dtime*24;
        if (dtime == 0) return 0;
        unit = DAY;
    }

    /* this is the hard part */

    if (unit == DAY) {
        /* set m and day to Jan 0, and readjust dtime */
        jday = d + monthjday[m-1];
        if (leap(y) && m > 2) jday++;
            dtime += jday;
/*
        while (dtime < 0) {
            y--;
	    dtime += 365 + leap(y);
        }
*/
	/* one year chunks */
    while (dtime > 365 + leap(y)) {
        dtime -= (365 + leap(y));
        y++;
    }

	/* calculate the month and day */

    if (leap(y) && dtime == FEB29) {
        m = 2;
        d = 29;
    }
    else {
        if (leap(y) && dtime > FEB29) dtime--;
        for (i = 11; monthjday[i] >= dtime; --i);
        m = i + 1;
        d = dtime - monthjday[i];
    }
    *year = y;
    *month = m;
    *day = d;
    return 0;
   }
   fprintf(stderr,"add_time: undefined time unit %d\n", unit);
   return 1;
}

/**
 * Subtracts a time interval from a time code.
 * 
 * @param year Pointer to year.
 * @param month Pointer to month.
 * @param day Pointer to day.
 * @param hour Pointer to hour.
 * @param minute Pointer to minute.
 * @param second Pointer to second.
 * @param dtime Time interval.
 * @param unit Time unit.
 * 
 * @return
 * - 0 Success.
 * - 1 Invalid time unit.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */

int sub_dt(int *year, int *month, int *day, int *hour, int *minute, int *second, int dtime, int unit) {

    int y, m, d, h, mm, s, jday, i, days_in_month;

    y = *year;
    m = *month;
    d = *day;
    h = *hour;
    mm = *minute;
    s = *second;
    if (unit == 255 || unit == -1) return 0;    // no valid time unit

    if (unit == YEAR) {
        *year = y - dtime;
        return 0;
    }
    if (unit == DECADE) {
        *year =  y - (10 * dtime);
        return 0;
    }
    if (unit == CENTURY) {
        *year = y - (100 * dtime);
        return 0;
    }
    if (unit == NORMAL) {
        *year = y - (30 * dtime);
        return 0;
    }
    if (unit == MONTH) {
        dtime += (12-m);
        *year = y = y - (dtime / 12);
        *month = m = 12 - (dtime % 12);
        /* check if date code if valid */
        days_in_month = monthjday[m] - monthjday[m-1];
        if (m == 2 && leap(y)) {
            days_in_month++;
        }
        if (days_in_month < d) *day = days_in_month;

        return 0;
    }
    if (unit == SECOND) {
        dtime += (59 - s);
        *second = s = 59 - (dtime % 60);
        dtime = dtime / 60;
        if (dtime == 0) return 0;
        unit = MINUTE;
    }
    if (unit == MINUTE) {
        dtime += (59 - mm);
        *minute = mm = 59 - dtime % 60;
        dtime = dtime / 60;
        if (dtime == 0) return 0;
        unit = HOUR;
    }
    if (unit == HOUR3) {
        dtime *= 3;
        unit = HOUR;
    }
    if (unit == HOUR6) {
        dtime *= 6;
        unit = HOUR;
    }
    if (unit == HOUR12) {
        dtime *= 12;
        unit = HOUR;
    }
    if (unit == HOUR) {
        dtime += (23-h);
        *hour = h = 23 - dtime%24;
        dtime = dtime / 24; 
        if (dtime == 0) return 0;
        unit = DAY;
    }

    /* this is the hard part */

    if (unit == DAY) {
        /* set m and day to Jan 0, and readjust dtime */
        jday = d + monthjday[m-1];
        if (leap(y) && m > 2) jday++;

        while (dtime >= jday) {
            // set date to dec 31 of previous year
            y = y - 1;
            m = 12;
            d = 31;
            dtime -= jday;
            jday = 365 + leap(y);
        }

        jday -= dtime;

        /* calculate the month and day */

        if (leap(y) && jday == FEB29) {
            m = 2;
            d = 29;
        }
        else {
            if (leap(y) && jday > FEB29) jday--;
            for (i = 11; monthjday[i] >= jday; --i);
            m = i + 1;
            d = jday - monthjday[i];
        }
        *year = y;
        *month = m;
        *day = d;
        return 0;
   }
   fprintf(stderr,"add_time: undefined time unit %d\n", unit);
   return 1;
}

/**
 * Reads a GRIB time code (year/month/day.../second) into separate variables.
 * 
 * @param p Pointer to GRIB time code bytes.
 * @param year Pointer to year.
 * @param month Pointer to month.
 * @param day Pointer to day.
 * @param hour Pointer to hour.
 * @param minute Pointer to minute.
 * @param second Pointer to second.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int get_time(unsigned char *p, int *year, int *month, int *day, int *hour, int *minute, int *second) {
    *year = (p[0] << 8) | p[1];
    p += 2;
    *month = (int) *p++;
    *day = (int) *p++;
    *hour = (int) *p++;
    *minute = (int) *p++;
    *second = (int) *p;
    return 0;
}

/**
 * Reads a GRIB time code (year/month/day.../second) into a full_date struct.
 * 
 * @param p Pointer to GRIB time code bytes.
 * @param date Pointer to full_date struct.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int Get_time(unsigned char *p, struct full_date *date) {
    date->year = (p[0] << 8) | p[1];
    date->month = p[2];
    date->day = p[3];
    date->hour = p[4];
    date->minute = p[5];
    date->second = p[6];
    return 0;
}

/**
 * Saves time code variables into GRIB time code bytes.
 * 
 * Inverse of get_time (saves time code in PDS).
 * 
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @param hour Hour.
 * @param minute Minute.
 * @param second Second.
 * @param p Pointer to output GRIB time code bytes.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int save_time(int year, int month, int day, int hour, int minute, int second, unsigned char *p) {

    *p++ = (unsigned char) (year >> 8) & 255;
    *p++ = (unsigned char) year & 255;
    *p++ = (unsigned char) month;
    *p++ = (unsigned char) day;
    *p++ = (unsigned char) hour;
    *p++ = (unsigned char) minute;
    *p++ = (unsigned char) second;
    return 0;
}

/*
   inverse of get_time .. save struct *full_date  in PDS
 */

/**
 * Saves a full_date struct into GRIB time code bytes.
 * 
 * Inverse of get_time (saves time code in PDS).
 * 
 * @param date Pointer to full_date struct.
 * @param p Pointer to output GRIB time code bytes.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2006
 */
int Save_time(struct full_date *date, unsigned char *p) {

    *p++ = (unsigned char) (date->year >> 8) & 255;
    *p++ = (unsigned char) date->year & 255;
    *p++ = (unsigned char) date->month;
    *p++ = (unsigned char) date->day;
    *p++ = (unsigned char) date->hour;
    *p++ = (unsigned char) date->minute;
    *p++ = (unsigned char) date->second;
    return 0;
}

/**
 * Compares two time codes.
 * 
 * @param year0 First year.
 * @param month0 First month.
 * @param day0 First day.
 * @param hour0 First hour.
 * @param minute0 First minute.
 * @param second0 First second.
 * @param year1 Second year.
 * @param month1 Second month.
 * @param day1 Second day.
 * @param hour1 Second hour.
 * @param minute1 Second minute.
 * @param second1 Second second.
 * 
 * @return 
 * - First < Second: -1
 * - First == Second: 0
 * - First > Second: 1
 *
 * @author Wesley Ebisuzaki @date 01/2006
 */
int cmp_time(int year0, int month0, int day0, int hour0, int minute0, int second0, int year1, int month1, int day1, int hour1, int minute1, int second1) {

    if (year0 < year1) return -1;
    if (year0 > year1) return 1;
    if (month0 < month1) return -1;
    if (month0 > month1) return 1;
    if (day0 < day1) return -1;
    if (day0 > day1) return 1;
    if (hour0 < hour1) return -1;
    if (hour0 > hour1) return 1;
    if (minute0 < minute1) return -1;
    if (minute0 > minute1) return 1;
    if (second0 < second1) return -1;
    if (second0 > second1) return 1;
    return 0;
}

/**
 * Compares two full_date structs.
 * 
 * @param date0 Pointer to the first full_date struct.
 * @param date1 Pointer to the second full_date struct.
 * 
 * @return 
 * - First < Second: -1
 * - First == Second: 0
 * - First > Second: 1
 *
 * @author Wesley Ebisuzaki @date 01/2006
 */
int Cmp_time(struct full_date *date0, struct full_date *date1) {

    if (date0->year < date1->year) return -1;
    if (date0->year > date1->year) return  1;

    if (date0->month < date1->month) return -1;
    if (date0->month > date1->month) return  1;

    if (date0->day < date1->day) return -1;
    if (date0->day > date1->day) return  1;

    if (date0->hour < date1->hour) return -1;
    if (date0->hour > date1->hour) return  1;

    if (date0->minute < date1->minute) return -1;
    if (date0->minute > date1->minute) return  1;

    if (date0->second < date1->second) return -1;
    if (date0->second > date1->second) return  1;

    return 0;
}
