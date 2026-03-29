#include "filetypes/sdelta.h"
#include <stdio.h>
#include <stdint.h>
#include <cpl_raylib.h>
#include <cpl_endian.h>
#include <stdlib.h>

// This file format is actually called MFS, or MultiFileStream.
// Stored in \Documents\SimCity\Games and probably some other directories [confirmation needed]
// SLDelta: Server Local Delta (Singleplayer server deltas)
// GDelta: Game Delta
// SDelta: Server Delta

// It seems to consist of a series of data blocks, each of which correspond to a certain offset in a file in some directory (possibly the statefile).
// It is likely expected that the data from each block be copied to the offset specified, thereby transmitting only the changes (deltas) between the server copy and the local copy.
// The true meaning of Game Delta remains unknown.

typedef struct SDelta2 {
    int32_t startOffset;
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
