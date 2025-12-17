/**
 * This tests the utility functions in addtime.c.
 * 
 * Alyson Stahl
 */

#include <stdio.h>
#include "wgrib2.h"
#include "CodeTable4_4.h"

int
main()
{
    printf("Testing check_datecode()...\n");
    {
        int ret;
        /* check_datecode(int year, int month, int day) */

        /* Invalid year */
        ret = check_datecode(-1, 6, 15);
        if (ret != 1) return 10;

        /* Invalid month */
        ret = check_datecode(2024, 13, 15);
        if (ret != 2) return 11;

        ret = check_datecode(2024, 0, 15);
        if (ret != 2) return 12;

        /* Invalid day */
        ret = check_datecode(2024, 6, -1);
        if (ret != 3) return 13;

        ret = check_datecode(2024, 6, 32);
        if (ret != 3) return 14;

        /** Invalid day for non leap year. */
        ret = check_datecode(2023, 2, 29);
        if (ret != 3) return 15;

        /** Valid day for leap year. */
        ret = check_datecode(2024, 2, 29);
        if (ret != 0) return 16;

        /** Invalid day for another non leap year. 
         * Want to test all branches of static function leap(). 
         */
        ret = check_datecode(1900, 2, 29);
        if (ret != 3) return 17;
    }
    printf("ok!\n");
    printf("Testing check_time()...\n");
    {
        int ret;
        /* check_time(int year, int month, int day, int hour, int minute, int second) */

        /* Invalid hour */
        ret = check_time(2024, 6, 15, -1, 0, 0);
        if (ret != 4) return 20;

        ret = check_time(2024, 6, 15, 24, 0, 0);
        if (ret != 4) return 21;

        /* Invalid minute */
        ret = check_time(2024, 6, 15, 12, -1, 0);
        if (ret != 5) return 22;

        ret = check_time(2024, 6, 15, 12, 60, 0);
        if (ret != 5) return 23;

        /* Invalid second */
        ret = check_time(2024, 6, 15, 12, 0, -1);
        if (ret != 6) return 24;

        ret = check_time(2024, 6, 15, 12, 0, 60);
        if (ret != 6) return 25;

        /* Valid time */
        ret = check_time(2024, 6, 15, 12, 30, 45);
        if (ret != 0) return 26;
    }
    printf("ok!\n");
    printf("Testing save_time() and get_time()...\n");
    {
        int ret;
        int year = 2024, month = 6, day = 15, hour = 12, minute = 30, second = 45;
        int out_yr, out_mon, out_day, out_hr, out_min, out_sec;
        unsigned char buf[7];

        /* Save time into buffer */
        ret = save_time(year, month, day, hour, minute, second, buf);
        if (ret != 0) return 30;

        /* Get time from buffer and check result. */
        ret = get_time(buf, &out_yr, &out_mon, &out_day, &out_hr, &out_min, &out_sec);
        if (ret != 0) return 31;
        if (year != out_yr || month != out_mon || day != out_day ||
            hour != out_hr || minute != out_min || second != out_sec) return 32;
    }
    printf("ok!\n");
    printf("Testing Save_time() and Get_time()...\n");
    {
        int ret;
        struct full_date date, out_date;
        unsigned char buf[7];

        date.year = 2024;
        date.month = 6;
        date.day = 15;
        date.hour = 12;
        date.minute = 30;
        date.second = 45;

        /* Save time into buffer */
        ret = Save_time(&date, buf);
        if (ret != 0) return 32;

        /* Get time from buffer and check result. */
        ret = Get_time(buf, &out_date);
        if (ret != 0) return 33;
        if (date.year != out_date.year || date.month != out_date.month || date.day != out_date.day ||
            date.hour != out_date.hour || date.minute != out_date.minute || date.second != out_date.second) return 34;
    }
    printf("ok!\n");
    printf("Testing cmp_time()...\n");
    {
        int ret;
        int year = 2024, month = 6, day = 15, hour = 12, minute = 30, second = 45;

        /** Equal dates. */
        ret = cmp_time(year, month, day, hour, minute, second,
                       year, month, day, hour, minute, second);
        if (ret != 0) return 40;

        /** Different years. */
        ret = cmp_time(year - 1, month, day, hour, minute, second,
                       year, month, day, hour, minute, second);
        if (ret != -1) return 41;

        ret = cmp_time(year, month, day, hour, minute, second,
                       year - 1, month, day, hour, minute, second);
        if (ret != 1) return 42;

        /** Different months. */
        ret = cmp_time(year, month - 1, day, hour, minute, second,
                       year, month, day, hour, minute, second);
        if (ret != -1) return 43;

        ret = cmp_time(year, month, day, hour, minute, second,
                       year, month - 1, day, hour, minute, second);
        if (ret != 1) return 44;

        /** Different days. */
        ret = cmp_time(year, month, day - 1, hour, minute, second,
                       year, month, day, hour, minute, second);
        if (ret != -1) return 45;

        ret = cmp_time(year, month, day, hour, minute, second,
                       year, month, day - 1, hour, minute, second);
        if (ret != 1) return 46;

        /** Different hours. */
        ret = cmp_time(year, month, day, hour - 1, minute, second,
                       year, month, day, hour, minute, second);
        if (ret != -1) return 47;

        ret = cmp_time(year, month, day, hour, minute, second,
                       year, month, day, hour - 1, minute, second);
        if (ret != 1) return 48;

        /** Different minutes. */
        ret = cmp_time(year, month, day, hour, minute - 1, second,
                       year, month, day, hour, minute, second);
        if (ret != -1) return 49;

        ret = cmp_time(year, month, day, hour, minute, second,
                       year, month, day, hour, minute - 1, second);
        if (ret != 1) return 50;

        /** Different seconds. */
        ret = cmp_time(year, month, day, hour, minute, second - 1,
                       year, month, day, hour, minute, second);
        if (ret != -1) return 51;

        ret = cmp_time(year, month, day, hour, minute, second,
                       year, month, day, hour, minute, second - 1);
        if (ret != 1) return 52;
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}