/**
 * This is a test for the wgrib2 project. This test is for all 
 * of the same_sec*() routines in test_sec.c.
 * 
 * Alyson Stahl, 3/2026
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"

int
main()
{
    printf("Testing same_sec0()...\n");
    {
        unsigned char sec0_a[16] = {
            'G', 'R', 'I', 'B', 
            0, 0,                   /* reserved */
            0, 2,                   /* discipline = 0, edition = 2 */
            0, 0, 0, 0, 0, 0, 0, 0  /* total length (not checked) */
        };
        unsigned char sec0_b[16] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        /**
         * same_sec0() does not compare beyond byte 8 (total length). 
         * Test will break if same_sec0() ever compares all 16 bytes.
         */
        for (int i = 0; i < 8; i++) sec0_b[i] = sec0_a[i];
        for (int i = 8; i < 16; i++) sec0_b[i] = 255;
        
        sec_a[0] = sec0_a;
        sec_b[0] = sec0_b;

        if (!same_sec0(sec_a, sec_b)) {
            printf("same_sec0: sections should be the same\n");
            return 1;
        }

        for (int i = 0; i < 8; i++) {
            sec0_b[i] = 255;
            if (same_sec0(sec_a, sec_b)) {
                printf("same_sec0: sections should be different at byte %d\n", i);
                return 1;
            }
            sec0_b[i] = sec0_a[i];
        }
    }
    printf("Testing same_sec0_not_var()...\n");
    {
        unsigned char sec0_a[16] = {
            'G', 'R', 'I', 'B', 
            0, 0,                   /* reserved */
            0, 2,                   /* discipline = 0, edition = 2 */
            0, 0, 0, 0, 0, 0, 0, 0  /* total length (not checked) */
        };
        unsigned char sec0_b[16] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        /**
         * same_sec0() does not compare beyond byte 8 (total length). 
         * Test will break if same_sec0() ever compares all 16 bytes.
         */
        for (int i = 0; i < 8; i++) sec0_b[i] = sec0_a[i];
        for (int i = 8; i < 16; i++) sec0_b[i] = 255;
        
        sec_a[0] = sec0_a;
        sec_b[0] = sec0_b;

        if (!same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: sections should be the same\n");
            return 1;
        }

        /* different edition, should be skipped */
        sec0_b[7] = 1;
        if (!same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: different edition should return 1\n");
            return 1;
        }
        sec0_b[7] = sec0_a[7];

        for (int i = 0; i < 8; i++) {
            if (i == 7) continue; /* ignores GRIB variable */
            sec0_b[i] = 255;
            if (same_sec0_not_var(1, sec_a, sec_b)) {
                printf("same_sec0_not_var: sections should be different at byte %d\n", i);
                return 1;
            }
            sec0_b[i] = sec0_a[i];
        }
    }
    printf("Testing same_sec1()...\n");
    {
        unsigned char sec1_a[21] = {
            0, 0, 0, 21,    /* Section length */
            1,              /* Section number */
            0, 7,           /* Center */
            0, 14,          /* Subcenter */
            2, 0,           /* Master table and local table */
            0,              /* Sig. of ref. time*/
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0, /* Month, day, hour, minute, second */
            0, 0            /* Prod status & type of processed data */
        };
        unsigned char sec1_b[21] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 21; i++) sec1_b[i] = sec1_a[i];

        sec_a[1] = sec1_a;
        sec_b[1] = sec1_b;

        if (!same_sec1(sec_a, sec_b)) {
            printf("same_sec1: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec1_b[3] = 20; 
        if (same_sec1(sec_a, sec_b)) {
            printf("same_sec1: different section length should return 0\n");
            return 1;
        }
        sec1_b[3] = sec1_a[3];

        for (int i = 4; i < 21; i++) {
            sec1_b[i] = 255;
            if (same_sec1(sec_a, sec_b)) {
                printf("same_sec1: sections should be different at byte %d\n", i);
                return 1;
            }
            sec1_b[i] = sec1_a[i];
        }
    }
    printf("testing same_sec1_not_time()...\n");
    {
        unsigned char sec1_a[21] = {
            0, 0, 0, 21,    /* Section length */
            1,              /* Section number */
            0, 7,           /* Center */
            0, 14,          /* Subcenter */
            2, 0,           /* Master table and local table */
            0,              /* Sig. of ref. time*/
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0, /* Month, day, hour, minute, second */
            0, 0            /* Prod status & type of processed data */
        };
        unsigned char sec1_b[21] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 21; i++) sec1_b[i] = sec1_a[i];

        sec_a[1] = sec1_a;
        sec_b[1] = sec1_b;

        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec1_b[3] = 20; 
        if (same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different section length should return 0\n");
            return 1;
        }
        sec1_b[3] = sec1_a[3];

        for (int i = 4; i < 21; i++) {
            if (i >= 12 && i < 19) continue; /* ignores time stamp */
            sec1_b[i] = 255;
            if (same_sec1_not_time(1, sec_a, sec_b)) {
                printf("same_sec1_not_time: sections should be different at byte %d\n", i);
                return 1;
            }
            sec1_b[i] = sec1_a[i];
        }

        /* different year */
        sec1_b[12] = (2024 >> 8) & 0xff;
        sec1_b[13] = 2024 & 0xff;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different year should return 1\n");
            return 1;
        }
        sec1_b[12] = sec1_a[12];
        sec1_b[13] = sec1_a[13];

        /* different month */
        sec1_b[14] = 4;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different month should return 1\n");
            return 1;
        }
        sec1_b[14] = sec1_a[14];

        /* different day */
        sec1_b[15] = 20;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different day should return 1\n");
            return 1;
        }
        sec1_b[15] = sec1_a[15];

        /* different hour */
        sec1_b[16] = 18;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different hour should return 1\n");
            return 1;
        }
        sec1_b[16] = sec1_a[16];

        /* different minute */
        sec1_b[17] = 30;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different minute should return 1\n");
            return 1;
        }
        sec1_b[17] = sec1_a[17];

        /* different second */
        sec1_b[18] = 45;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different second should return 1\n");
            return 1;
        }
        sec1_b[18] = sec1_a[18];

    }
    printf("Testing same_sec1_not_var()...\n");
    {
        unsigned char sec1_a[21] = {
            0, 0, 0, 21,    /* Section length */
            1,              /* Section number */
            0, 7,           /* Center */
            0, 14,          /* Subcenter */
            2, 0,           /* Master table and local table */
            0,              /* Sig. of ref. time*/
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0, /* Month, day, hour, minute, second */
            0, 0            /* Prod status & type of processed data */
        };
        unsigned char sec1_b[21] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        /**
         * same_sec1_not_var() does not compare beyond byte 12. 
         * Test will break if same_sec1_not_var() ever compares all 21 bytes.
         */
        for (int i = 0; i < 12; i++) sec1_b[i] = sec1_a[i];
        for (int i = 12; i < 21; i++) sec1_b[i] = 255;

        sec_a[1] = sec1_a;
        sec_b[1] = sec1_b;

        if (!same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec1_b[3] = 20;
        if (same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: different section length should return 0\n");
            return 1;
        }
        sec1_b[3] = sec1_a[3];

        for (int i = 4; i < 12; i++) {
            if (i == 9 || i == 10) continue; /* ignores master table and local table */
            sec1_b[i] = 255;
            if (same_sec1_not_var(1, sec_a, sec_b)) {
                printf("same_sec1_not_var: sections should be different at byte %d\n", i);
                return 1;
            }
            sec1_b[i] = sec1_a[i];
        }

        /* different master table */
        sec1_b[9] = 0;
        if (!same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: different master table should return 1\n");
            return 1;
        }
        sec1_b[9] = sec1_a[9];

        /* different local table */
        sec1_b[10] = 255;
        if (!same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: different local table should return 1\n");
            return 1;
        }
        sec1_b[10] = sec1_a[10];
    }
    printf("Testing same_sec2()...\n");
    {
        unsigned char sec2_a[6] = {
            0, 0, 0, 6, /* Section length */
            2,          /* Section number */
            0           /* Local Use */
        };
        unsigned char sec2_b[6] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 6; i++) sec2_b[i] = sec2_a[i];

        sec_a[2] = sec2_a;
        sec_b[2] = sec2_b;

        if (!same_sec2(sec_a, sec_b)) {
            printf("same_sec2: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec2_b[3] = 5;
        if (same_sec2(sec_a, sec_b)) {
            printf("same_sec2: different section length should return 0\n");
            return 1;
        }
        sec2_b[3] = sec2_a[3];

        for (int i = 4; i < 6; i++) {
            sec2_b[i] = 255;
            if (same_sec2(sec_a, sec_b)) {
                printf("same_sec2: sections should be different at byte %d\n", i);
                return 1;
            }
            sec2_b[i] = sec2_a[i];
        }
    }
    printf("Testing same_sec3()...\n");
    {
        unsigned char sec3_a[15] = {
            0, 0, 0, 15,    /* Section length */
            3,              /* Section number */
            0,              /* Source of GDT */
            0, 0, 0, 0,     /* Number of data points */
            0,              /* Num octets for optional list */
            0,              /* Interpretation of optional list */
            0, 0,           /* Grid definition template number */
            0               /* Template */
        };
        unsigned char sec3_b[15] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 15; i++) sec3_b[i] = sec3_a[i];

        sec_a[3] = sec3_a;
        sec_b[3] = sec3_b;

        if (!same_sec3(sec_a, sec_b)) {
            printf("same_sec3: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec3_b[3] = 14;
        if (same_sec3(sec_a, sec_b)) {
            printf("same_sec3: different section length should return 0\n");
            return 1;
        }
        sec3_b[3] = sec3_a[3];

        for (int i = 6; i < 15; i++) {
            sec3_b[i] = 255;
            if (same_sec3(sec_a, sec_b)) {
                printf("same_sec3: sections should be different at byte %d\n", i);
                return 1;
            }
            sec3_b[i] = sec3_a[i];
        }
    }
    printf("Testing same_sec4()...\n");
    {
        unsigned char sec4_a[10] = {
            0, 0, 0, 20,    /* Section length */
            4,              /* Section number */
            0, 0,           /* Num coordinate values after template */
            0, 0,           /* Product definition template number */
            0               /* Template */
        };
        unsigned char sec4_b[10] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 10; i++) sec4_b[i] = sec4_a[i];

        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        if (!same_sec4(sec_a, sec_b)) {
            printf("same_sec4: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec4_b[3] = 19;
        if (same_sec4(sec_a, sec_b)) {
            printf("same_sec4: different section length should return 0\n");
            return 1;
        }
        sec4_b[3] = sec4_a[3];

        for (int i = 10; i < 20; i++) {
            sec4_b[i] = 255;
            if (same_sec4(sec_a, sec_b)) {
                printf("same_sec4: sections should be different at byte %d\n", i);
                return 1;
            }
            sec4_b[i] = sec4_a[i];
        }
    }
    printf("SUCCESS!\n");
    return 0;
}