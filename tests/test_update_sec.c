/**
 * This tests the utility functions update_sec3() and update_sec4().
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include <setjmp.h>
#include "wgrib2.h"

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing update_sec3().\n");
    printf("Testing simple call of update_sec3()...\n");
    {

        unsigned int i;
        unsigned char *sec[10] = {0};
        unsigned char old_sec3[8] = {0};
        unsigned char new_sec3_buf[8] = {0};

        unsigned int sec3_len = 8;
        new_sec3_buf[0] = (sec3_len >> 24) & 0xFF;
        new_sec3_buf[1] = (sec3_len >> 16) & 0xFF;
        new_sec3_buf[2] = (sec3_len >> 8) & 0xFF;
        new_sec3_buf[3] = sec3_len & 0xFF;

        new_sec3_buf[4] = 10;
        new_sec3_buf[5] = 20;
        new_sec3_buf[6] = 30;
        new_sec3_buf[7] = 40;

        sec[3] = old_sec3;

        update_sec3(sec, new_sec3_buf);

        if (sec[3] == old_sec3) {
            printf("update_sec3() did not replace old section pointer.\n");
            return 10;
        }
        
        for (i = 0; i < sec3_len; i++) {
            if (sec[3][i] != new_sec3_buf[i]) {
                printf("update_sec3() did not copy section correctly.\n");
                return 11;
            }
        }

    }
    printf("Testing with section that is too small. Should fail...\n");
    {
        unsigned char *sec[10] = {0};
        unsigned char old_sec3[8] = {0};
        unsigned char small_sec3[8] = {0};

        unsigned int sec3_len_small = 5;
        small_sec3[0] = (sec3_len_small >> 24) & 0xFF;
        small_sec3[1] = (sec3_len_small >> 16) & 0xFF;
        small_sec3[2] = (sec3_len_small >> 8) & 0xFF;
        small_sec3[3] = sec3_len_small & 0xFF;

        sec[3] = old_sec3;

        if (setjmp(fatal_err) == 0) {
            update_sec3(sec, small_sec3);
            printf("update_sec3 did not fail for small section\n");
            return 12;
        } else {
            if (sec[3] != old_sec3) {
                printf("update_sec3 modified section pointer on error\n");
                return 13;
            }
        }
    }
    printf("Testing update_sec4().\n");
    printf("Testing simple call of update_sec4()...\n");
    {
        unsigned int i;
        unsigned char *sec[10] = {0};
        unsigned char old_sec4[16] = {0};
        unsigned char new_sec4_buf[16] = {0};

        unsigned int sec4_len = 16;
        new_sec4_buf[0] = (sec4_len >> 24) & 0xFF;
        new_sec4_buf[1] = (sec4_len >> 16) & 0xFF;
        new_sec4_buf[2] = (sec4_len >> 8) & 0xFF;
        new_sec4_buf[3] = sec4_len & 0xFF;

        for (i = 4; i < sec4_len; i++) {
            new_sec4_buf[i] = (unsigned char)(i * 3);
        }

        sec[4] = old_sec4;

        update_sec4(sec, new_sec4_buf);

        if (sec[4] == old_sec4) {
            printf("update_sec4() did not replace old section pointer.\n");
            return 14;
        }

        for (i = 0; i < sec4_len; i++) {
            if (sec[4][i] != new_sec4_buf[i]) {
                printf("update_sec4() did not copy section correctly.\n");
                return 15;
            }
        }

    }
    printf("Testing simple call of update_sec4()...\n");
    {
        unsigned char *sec[10] = {0};
        unsigned char old_sec4[16] = {0};
        unsigned char large_sec4[16] = {0};

        unsigned int sec4_len_large = 0xFFFFFFFFu;
        large_sec4[0] = (sec4_len_large >> 24) & 0xFF;
        large_sec4[1] = (sec4_len_large >> 16) & 0xFF;
        large_sec4[2] = (sec4_len_large >> 8) & 0xFF;
        large_sec4[3] = sec4_len_large & 0xFF;

        sec[4] = old_sec4;

        if (setjmp(fatal_err) == 0) {
            update_sec4(sec, large_sec4);
            printf("update_sec4() did not fail for oversized section.\n");
            return 16;
        } else {
            if (sec[4] != old_sec4) {
                printf("update_sec4() modified section pointer on oversized error.\n");
                return 17;
            }
        }

    }
    printf("Testing simple call of update_sec4()...\n");
    {
        unsigned char *sec[10] = {0};
        unsigned char old_sec4[16] = {0};
        unsigned char small_sec4[16] = {0};

        unsigned int sec4_len_small = 8;
        small_sec4[0] = (sec4_len_small >> 24) & 0xFF;
        small_sec4[1] = (sec4_len_small >> 16) & 0xFF;
        small_sec4[2] = (sec4_len_small >> 8) & 0xFF;
        small_sec4[3] = sec4_len_small & 0xFF;

        sec[4] = old_sec4;

        if (setjmp(fatal_err) == 0) {
            update_sec4(sec, small_sec4);
            printf("update_sec4() did not fail for undersized section.\n");
            return 18;
        } else {
            if (sec[4] != old_sec4) {
                printf("update_sec4() modified section pointer on undersized error.\n");
                return 19;
            }
        }

    }
    printf("SUCCESS!\n");
    return 0;
}