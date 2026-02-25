/**
 * This is a test for the wgrib2 project. This test is for addtime.c.
 * 
 * Alyson Stahl, 12/2025
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
    printf("Testing Cmp_time()...\n");
    {
        int ret;
        struct full_date date0 = { 2024, 6, 15, 12, 30, 45 };
        struct full_date date1 = { 2024, 6, 15, 12, 30, 45 };

        /** Equal dates. */
        ret = Cmp_time(&date0, &date1);
        if (ret != 0) return 53;

        /** Different years. */
        date0.year = 2023;
        ret = Cmp_time(&date0, &date1);
        if (ret != -1) return 54;

        ret = Cmp_time(&date1, &date0);
        if (ret != 1) return 55;

        /** Different months. */
        date0.year = 2024;
        date0.month = 5;
        ret = Cmp_time(&date0, &date1);
        if (ret != -1) return 56;

        ret = Cmp_time(&date1, &date0);
        if (ret != 1) return 57;

        /** Different days. */
        date0.month = 6;
        date0.day = 14;
        ret = Cmp_time(&date0, &date1);
        if (ret != -1) return 58;

        ret = Cmp_time(&date1, &date0);
        if (ret != 1) return 59;

        /** Different hours. */
        date0.day = 15;
        date0.hour = 11;
        ret = Cmp_time(&date0, &date1);
        if (ret != -1) return 60;

        ret = Cmp_time(&date1, &date0);
        if (ret != 1) return 61;

        /** Different minutes. */
        date0.hour = 12;
        date0.minute = 29;
        ret = Cmp_time(&date0, &date1);
        if (ret != -1) return 62;

        ret = Cmp_time(&date1, &date0);
        if (ret != 1) return 63;

        /** Different seconds. */
        date0.minute = 30;
        date0.second = 44;
        ret = Cmp_time(&date0, &date1);
        if (ret != -1) return 64;

        ret = Cmp_time(&date1, &date0);
        if (ret != 1) return 65;
    }
    printf("ok!\n");
    printf("Testing add_time()...\n");
    {
        int ret;
        int year0 = 2024, month0 = 6, day0 = 15, hour0 = 12, minute0 = 30, second0 = 45, dtime = 1;
        int year = year0, month = month0, day = day0, hour = hour0, minute = minute0, second = second0;
        int unit[] = { MINUTE, HOUR, DAY, MONTH, YEAR, DECADE, NORMAL, CENTURY, HOUR3, HOUR6, HOUR12, SECOND };
        int n_units = sizeof(unit) / sizeof(unit[0]);

        /** No valid time unit (should return 0) - check for + and - dtime */
        ret = add_time(&year, &month, &day, &hour, &minute, &second, dtime, 255);
        if (ret != 0) return 70;

        ret = add_time(&year, &month, &day, &hour, &minute, &second, -dtime, 255);
        if (ret != 0) return 71;

        /** Invalid time unit (should return 1) - check for + and - dtime*/
        ret = add_time(&year, &month, &day, &hour, &minute, &second, dtime, 15);
        if (ret != 1) return 72;

        ret = add_time(&year, &month, &day, &hour, &minute, &second, -dtime, 15);
        if (ret != 1) return 73;

        /** Valid time unit with dt = 0. */
        ret = add_time(&year, &month, &day, &hour, &minute, &second, 0, MINUTE);
        if (ret != 0) return 74;

        /** Add then subtract dtime from the date. Should result in the original date. */
        for (size_t i = 0; i < n_units; i++) {
            ret = add_time(&year, &month, &day, &hour, &minute, &second, dtime, unit[i]);
            if (ret != 0) {
                printf("add_time() failed for +dtime with unit %d\n", unit[i]);
                return 75;
            }

            ret = add_time(&year, &month, &day, &hour, &minute, &second, -dtime, unit[i]);
            if (ret != 0) {
                printf("add_time() failed for -dtime with unit %d\n", unit[i]);
                return 76;
            }

            if (year != year0 || month != month0 || day != day0 || hour != hour0 || 
                minute != minute0 || second != second0) {
                printf("Test failed for unit %d\n", unit[i]);
                return 77;
            }
        }
    }
    printf("ok!\n");
    printf("Testing Add_time()...\n");
    {
        int ret;
        struct full_date date = { 2024, 1, 1, 12, 30, 45 };
        struct full_date exp_date = { 2024, 1, 1, 12, 30, 45 };
        int dtime = 59, unit = DAY;

        /** Test with dtime = 0. Should not change the date. */
        ret = Add_time(&date, 0, unit);
        if (ret != 0) return 80;

        if (date.year != exp_date.year || date.month != exp_date.month || date.day != exp_date.day ||
            date.hour != exp_date.hour || date.minute != exp_date.minute || date.second != exp_date.second) {
            return 81;
        }

        /** Test some edge cases with leap year and unit = DAY. */
        exp_date.month = 2;
        exp_date.day = 29;

        ret = Add_time(&date, dtime, unit);
        if (ret != 0) return 82;

        if (date.year != exp_date.year || date.month != exp_date.month || date.day != exp_date.day ||
            date.hour != exp_date.hour || date.minute != exp_date.minute || date.second != exp_date.second) {
            return 83;
        }

        date.month = 4;
        date.day = 28;

        ret = Add_time(&date, -dtime, unit);
        if (ret != 0) return 84;

        if (date.year != exp_date.year || date.month != exp_date.month || date.day != exp_date.day ||
            date.hour != exp_date.hour || date.minute != exp_date.minute || date.second != exp_date.second) {
            return 85;
        }
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}