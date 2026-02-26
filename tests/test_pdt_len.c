/**
 * This is a test for the wgrib2 project. This test is for pdt_len.c.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include "grb2.h"
#include <setjmp.h>
#include "wgrib2.h"

#define N_PDT_CASES 58
#define N_JMA_PDT_CASES 13

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing pdt_len().\n");
    printf("Testing pdt_len() with NULL GRIB section and unrecognized PDT. Should return -1...\n");
    {
        if ((pdt_len(NULL, 9999) != -1)) {
            return 10;
        }
    }
    printf("Testing pdt_len() with pdt = -1 in GRIB section. Should return -1...\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec4[64] = {0};
        unsigned char *sec[5];

        sec[0] = NULL;
        sec[1] = sec1;
        sec[2] = NULL;
        sec[3] = NULL;
        sec[4] = sec4;

        /* unrecognized PDT value in section 4 */
        sec4[7] = (unsigned char) (9999 >> 8);
        sec4[8] = (unsigned char) (9999 & 0xFF);

        if ((pdt_len(sec, -1) != -1)) {
            return 11;
        }
    }
    printf("Testing pdt_len() with all cases of pdt < 32768...\n");
    {
        int pdt_in[N_PDT_CASES] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20, 30, 31, 32, 
            33, 34, 35, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 53, 54, 55, 
            56, 57, 58, 59, 60, 61, 62, 63, 67, 68, 70, 71, 72, 73, 91, 254, 1000, 
            1001, 1002, 1100, 1101
        };
        
        int exp_pdt_len[N_PDT_CASES] = {
            50, 53, 52, 85, 81, 63, 51, 50, 74, 87,
            75, 77, 76, 109, 105, 53, 59, 30, 52, 61,
            64, 88, 53, 52, 55, 76, 79, 61, 66, 87,
            90, 74, 77, 63, 56, 59, 56, 58, 64, 67,
            59, 60, 84, 83, 83, 88, 88, 55, 58, 79,
            82, 87, 31, 38, 54, 51, 50, 66
        };

        unsigned char sec1[16] = {0};
        unsigned char sec4[64] = {0};
        unsigned char *sec[5];

        sec[0] = NULL;
        sec[1] = sec1;
        sec[2] = NULL;
        sec[3] = NULL;
        sec[4] = sec4;

        /* Number of coordinate values. */
        sec4[5] = 0;
        sec4[6] = 4;

        /* PDT value location */
        sec4[7] = 0xFF;
        sec4[8] = 0xFF;
        
        /* Number of contributing spectral bands (nb = 2). */
        sec4[13] = 2;  /* used by pdt 31 */
        sec4[22] = 2;  /* used by pdt 32,33,34 */
        sec4[14] = 2;  /* used by pdt 35 */

        /* Number of forecasts in cluster (nc = 1) */
        sec4[57] = 1; /* used by pdt 3 and 13 */
        sec4[53] = 1; /* used by pdt 4 and 14 */
        sec4[34] = 1; /* used by pdt 51 and 91 */

        /* Number of following function parameters (np = 1) */
        sec4[12] = 1; /* used by pdt 53 and 54 */
        sec4[19] = 1; /* used by pdt 57, 58, 67, and 68 */

        /* Statistical processing: set n = 1 where applicable. */
        sec4[41] = 1; /* used by pdt 8  */
        sec4[42] = 1; /* used by pdt 10 */
        sec4[43] = 1; /* used by pdt 12 and 42 */
        sec4[44] = 1; /* used by pdt 11 */
        sec4[46] = 1; /* used by pdt 43 and 72 */
        sec4[47] = 1; /* used by pdt 62 */
        sec4[49] = 1; /* used by pdt 73 */
        sec4[50] = 1; /* used by pdt 63 */
        sec4[51] = 1; /* used by pdt 61 */
        sec4[54] = 1; /* used by pdt 9 and 46 */
        sec4[55] = 1; /* used by pdt 34, 67, 68 */

        for (int i = 0; i < N_PDT_CASES; i++) {
            int actual_len;

            /* Set PDT value in section 4 for this test case. */
            sec4[7] = (unsigned char) (pdt_in[i] >> 8);
            sec4[8] = (unsigned char) (pdt_in[i] & 0xFF);

            actual_len = pdt_len(sec, -1);
            if (actual_len != exp_pdt_len[i]) {
                printf("pdt_len() failed for PDT %d: expected %d, got %d\n",
                        pdt_in[i], exp_pdt_len[i], actual_len);
                return 12;
            }
        }
    }
    printf("Testing pdt_len() with all cases where center is JMA...\n");
    {

        int pdt_in[N_JMA_PDT_CASES] = { 
            50000, 50008, 50010, 50011, 50009, 50012, 50020, 51020, 51021, 
            51022, 51122, 51123, 52020
        };

        int exp_pdt_len[N_JMA_PDT_CASES] = { 
            58, 98, 98, 98, 103, 82, 52, 60, 60, 80, 80, 81, 61 
        };

        unsigned char sec1[16] = {0};
        unsigned char sec3[32] = {0};
        unsigned char sec4[84] = {0};
        unsigned char *sec[5];

        sec[0] = NULL;
        sec[1] = sec1;
        sec[2] = NULL;
        sec[3] = sec3;
        sec[4] = sec4;

        sec1[5] = 0;
        sec1[6] = JMA1;

        sec4[82] = 0;
        sec4[83] = 1;

        /* Sets Nr for pdt 51022, 51122, and 51123 */
        sec3[12] = (unsigned char) (50120 >> 8);
        sec3[13] = (unsigned char) (50120 & 0xFF);
        sec3[18] = 0;
        sec3[19] = 0;
        sec3[20] = 0;
        sec3[21] = 1;

        /* Sets Fp and Ft for pdt 51123 */
        sec4[55] = 1;
        sec4[56] = 1;

        /* Number of coordinate values. */
        sec4[5] = 0;
        sec4[6] = 4;

        /* PDT value location */
        sec4[7] = 0xFF;
        sec4[8] = 0xFF;
        
        for (int i = 0; i < N_JMA_PDT_CASES; i++) {
            int actual_len;

            /* Set PDT value in section 4 for this test case. */
            sec4[7] = (unsigned char) (pdt_in[i] >> 8);
            sec4[8] = (unsigned char) (pdt_in[i] & 0xFF);

            actual_len = pdt_len(sec, -1);
            if (actual_len != exp_pdt_len[i]) {
                printf("pdt_len() failed for PDT %d: expected %d, got %d\n",
                        pdt_in[i], exp_pdt_len[i], actual_len);
                return 13;
            }
        }
    }
    printf("SUCCESS!\n");
    return 0;
}