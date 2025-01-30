#include "filetypes/sdelta.h"
#include <stdio.h>
#include <stdint.h>
#include <cpl_raylib.h>
#include <cpl_endian.h>
#include <stdlib.h>

typedef struct SDelta2 {

} SDelta2;

SDelta LoadSDeltaFile(const char *filename)
{
    SDelta sdelta = { 0 };

    FILE *f = fopen(filename, "rb");

    uint32_t headerVal;
    fread(&headerVal, 4, 1, f);

    if (headerVal != 4) TRACELOG(LOG_WARNING, "Header value is %#x\n", headerVal);

    int index = 0;

    while (!feof(f))
    {
        index++;
        uint32_t signature = 0;

        fread(&signature, 4, 1, f);
        signature = be32toh(signature);

        uint32_t size = 0;

        fread(&size, 4, 1, f);
        size = be32toh(size);
        if (!size) break;

        TRACELOG(LOG_DEBUG, "File %d:\n", index);
        TRACELOG(LOG_DEBUG, "Signature: %#x\n", signature);
        TRACELOG(LOG_DEBUG, "Size: %#x\n", size);

        uint8_t *data = malloc(size - 8);
        fread(data, 1, size - 8, f);

        SaveFileData(TextFormat("corrupted/%x-%x.mfs", signature, index), data, size - 8);

        free(data);
    }

    fclose(f);

    return sdelta;
}
