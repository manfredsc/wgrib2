/**
 * This is a test for the gmerge helper function rd_msg().
 * 
 * Alyson Stahl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int rd_msg(FILE *in, FILE *out);

int
main()
{
    printf("Testing gmerge helper function rd_msg()...\n");
    printf("EOF on first read. Should return -1.");
    {
        FILE *input, *output;
        int result;

        // Create empty file
        input = fopen("test_empty.tmp", "wb");
        fclose(input);
        
        input = fopen("test_empty.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);

        remove("test_empty.tmp");
        remove("test_output.tmp");

        if (result != -1) return 1;
    }
    printf("ok!\n");
    printf("Invalid GRIB header. Should return -1.");
    {
        FILE *input, *output;
        unsigned char bad_data[16]; 
        int result;
        
        memset(bad_data, 0, 16);
        strcpy((char*)bad_data, "BADHEADER");

        input = fopen("test_bad.tmp", "wb");
        fwrite(bad_data, 1, 16, input);
        fclose(input);
        
        input = fopen("test_bad.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);
        remove("test_bad.tmp");
        remove("test_output.tmp");

        if (result != -1) return 2;
    }
    printf("ok!\n");
    printf("Message too large (> 1GB). Should return -1.");
    {
        FILE *input, *output;
        unsigned char large_header[16]; 
        int result;
        
        memset(large_header, 0, 16);
        strcpy((char*)large_header, "GRIB");
        // Set message length to > 1GB
        large_header[7] = 0x40; // 1GB = 0x40000000

        input = fopen("test_large.tmp", "wb");
        fwrite(large_header, 1, 16, input);
        fclose(input);
        
        input = fopen("test_large.tmp", "rb");
        output = fopen("test_output.tmp", "wb");
        
        result = rd_msg(input, output);

        fclose(input);
        fclose(output);
        remove("test_large.tmp");
        remove("test_output.tmp");

        if (result != -1) return 3;
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}