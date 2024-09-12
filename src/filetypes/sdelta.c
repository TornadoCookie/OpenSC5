#include "filetypes/sdelta.h"
#include <stdio.h>
#include <cpl_endian.h>
#include <stdint.h>

typedef struct SDelta2 {

} SDelta2;

SDelta LoadSDeltaFile(const char *filename)
{
    SDelta sdelta = { 0 };

    FILE *f = fopen(filename, "rb");

    uint32_t headerVal;
    fread(&headerVal, 4, 1, f);

    if (headerVal != 0x00023720) printf("Header value is %#x\n", headerVal);

    for (int i = 0; i < 3; i++)
    {
        
    }

    fclose(f);

    return sdelta;
}
