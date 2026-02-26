/**
 * This is a test for the wgrib2 project. This test is for subtime.c.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include "wgrib2.h"
#include "CodeTable4_4.h"

# define NUM_TESTS 11
int
main()
{
    printf("Testing jday()...\n");
    {
        int day;
        
        /* Non-leap year */
        day = jday(2023, 3, 1);
        if (day != 60) {
            printf("Error: jday() returned %d, expected 60.\n", day);
            return 10;
        }

        /* Leap year, not century */
        day = jday(2024, 3, 1);
        if (day != 61) {
            printf("Error: jday() returned %d, expected 61.\n", day);
            return 11;
        }

        /* Century year that is not leap */
        day = jday(1900, 2, 28);
        if (day != 59) {
            printf("Error: jday() returned %d, expected 59.\n", day);
            return 12;
        }
        
    }
    printf("Testing num_days_in_month()...\n");
    {
        int days;

        /* month != 2: January 2023 has 31 days */        
        days = num_days_in_month(2023, 1);
        if (days != 31) {
            printf("Error: num_days_in_month() returned %d, expected 31.\n", days);
            return 13;
        }

        /* month == 2 and leap year: February 2024 has 29 days */
        days = num_days_in_month(2024, 2);
        if (days != 29) {
            printf("Error: num_days_in_month() returned %d, expected 29.\n", days);
            return 14;
        }
        
        /* month == 2 and non-leap year: February 2023 has 28 days */
        days = num_days_in_month(2023, 2);
        if (days != 28) {
            printf("Error: num_days_in_month() returned %d, expected 28.\n", days);
            return 15;
        }
    }
    printf("Testing sub_time().\n");
    {
        int year0[NUM_TESTS], year1[NUM_TESTS];
        int month0[NUM_TESTS], month1[NUM_TESTS];
        int day0[NUM_TESTS], day1[NUM_TESTS];
        int hour0[NUM_TESTS], hour1[NUM_TESTS];
        int minute0[NUM_TESTS], minute1[NUM_TESTS];
        int second0[NUM_TESTS], second1[NUM_TESTS];
        int exp_dtime[NUM_TESTS];
        int exp_unit[NUM_TESTS];

        for (int i = 0; i < NUM_TESTS; i++) {
            year0[i] = 2020;
            month0[i] = 1;
            day0[i] = 1;
            hour0[i] = 0;
            minute0[i] = 0;
            second0[i] = 0;

            year1[i] = 2021;
            month1[i] = 2;
            day1[i] = 2;
            hour1[i] = 2;
            minute1[i] = 30;
            second1[i] = 45;
        }

        /** Case 1: Equal dates. */
        year1[0] = year0[0];
        month1[0] = month0[0];
        day1[0] = day0[0];
        hour1[0] = hour0[0];
        minute1[0] = minute0[0];
        second1[0] = second0[0];
        exp_dtime[0] = 0;
        exp_unit[0] = YEAR;

        /** Case 2: Different years. */
        month1[1] = month0[1];
        day1[1] = day0[1];
        hour1[1] = hour0[1];
        minute1[1] = minute0[1];
        second1[1] = second0[1];
        exp_dtime[1] = 1;
        exp_unit[1] = YEAR;

        /** Case 3: Different years and months. */
        day1[2] = day0[2];
        hour1[2] = hour0[2];
        minute1[2] = minute0[2];
        second1[2] = second0[2];
        exp_dtime[2] = 13;
        exp_unit[2] = MONTH;

        /** Case 4: Different years, months, and days. */
        hour1[3] = hour0[3];
        minute1[3] = minute0[3];
        second1[3] = second0[3];
        exp_dtime[3] = 398;
        exp_unit[3] = DAY;

        /** Case 5: Different years, months, and days, with time1 < time0 (sign < 0). */
        year0[4] = 2021; year1[4] = 2020;
        month0[4] = 2; month1[4] = 1;
        day0[4] = 2; day1[4] = 1;
        hour1[4] = hour0[4];
        minute1[4] = minute0[4];
        second1[4] = second0[4];
        exp_dtime[4] = -398;
        exp_unit[4] = DAY;

        /** Case 6: Different years, months, days, and hours, where unit is HOUR. */
        minute1[5] = minute0[5];
        second1[5] = second0[5];
        exp_dtime[5] = 9554;
        exp_unit[5] = HOUR;

        /** Case 7: Different years, months, days, and hours, where unit is HOUR12. */
        hour1[6] = 12;
        minute1[6] = minute0[6];
        second1[6] = second0[6];
        exp_dtime[6] = 797;
        exp_unit[6] = HOUR12;

        /** Case 8: Different years, months, days, and hours, where unit is HOUR6. */
        hour1[7] = 18;
        minute1[7] = minute0[7];
        second1[7] = second0[7];
        exp_dtime[7] = 1595;
        exp_unit[7] = HOUR6;

        /** Case 9: Different years, months, days, and hours, where unit is HOUR3. */
        hour1[8] = 21;
        minute1[8] = minute0[8];
        second1[8] = second0[8];
        exp_dtime[8] = 3191;
        exp_unit[8] = HOUR3;

        /** Case 10: Different years, months, days, hours, and minutes. */
        second1[9] = second0[9];
        exp_dtime[9] = 573270;
        exp_unit[9] = MINUTE;
        
        /** Case 11: Different years, months, days, hours, minutes, and seconds. */
        exp_dtime[10] = 34396245;
        exp_unit[10] = SECOND;

        for (int i = 0; i < NUM_TESTS; i++) {
            int dtime;
            int unit;

            sub_time(year1[i], month1[i], day1[i], hour1[i], minute1[i], second1[i],
                    year0[i], month0[i], day0[i], hour0[i], minute0[i], second0[i],
                    &dtime, &unit);

            if (dtime != exp_dtime[i] || unit != exp_unit[i]) {
                printf("Error for test %d: returned dtime=%d, unit=%d; expected dtime=%d, unit=%d\n",
                        i, dtime, unit, exp_dtime[i], exp_unit[i]);
                return 16;
            }
        }
    }
    printf("SUCCESS!\n");
    return 0;
}