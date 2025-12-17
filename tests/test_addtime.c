/**
 * This tests the utility functions in addtime.c.
 * 
 * Alyson Stahl
 */

#include <stdio.h>
#include "wgrib2.h"
//#include "CodeTable4_4.h"

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
    printf("SUCCESS!\n");
    return 0;
}