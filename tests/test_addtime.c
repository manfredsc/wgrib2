/**
 * This tests the utility functions in addtime.c.
 * 
 * Alyson Stahl
 */

#include <stdio.h>

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
    printf("SUCCESS!\n");
    return 0;
}