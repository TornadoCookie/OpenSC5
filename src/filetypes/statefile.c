#include "filetypes/statefile.h"
#include <cpl_raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <cpl_endian.h>

typedef struct StateFileChunk {
    //uint32_t null;
    uint64_t signature;
    uint32_t size;
} StateFileChunk;

typedef struct StateFileHeader0 {
    uint32_t unknown[3];
    uint32_t unknown2Count;
} StateFileHeader0;

typedef struct StateFileHeader {
    uint32_t unknown3[6];
    uint32_t imageCount;
} StateFileHeader;

StateFile LoadStateFile(const char *filename)
{
    FILE *f = fopen(filename, "rb");

    StateFileHeader0 header0 = { 0 };
    
    fread(&header0, sizeof(StateFileHeader0), 1, f);

    if (header0.unknown2Count)
    {
        header0.unknown2Count = be32toh(header0.unknown2Count);
        printf("unknown 2 count: %d\n", header0.unknown2Count);
        fseek(f, 2 * header0.unknown2Count * 4, SEEK_CUR);  
    }

    StateFileHeader header = { 0 };
    fread(&header, sizeof(StateFileHeader), 1, f);

    header.imageCount = be32toh(header.imageCount);

    // TODO this assumes the file is already gzip decompressed.
    for (int i = 0; i < header.imageCount; i++)
    {
        StateFileChunk chunk = { 0 };

        fread(&chunk, sizeof(StateFileChunk), 1, f);

        //if (chunk.null != 0) printf("expected NULL, got %#x\n", chunk.null);
        printf("signature: %#lx\n", chunk.signature);

        chunk.size = be32toh(chunk.size);

        printf("size: %d\n", chunk.size);
        
        uint32_t *imgData = malloc(chunk.size * 4);
        fread(imgData, 4, chunk.size, f);

        for (int j = 0; j < chunk.size; j++)
        {
            Color *col = ((Color*)imgData)+j;
            uint32_t asint = imgData[j];
            asint = be32toh(asint);
            uint16_t as16 = asint & 0xFFFF;
            float asflt = as16;
            asflt /= 0xFFFF;
            asflt *= 255;
            uint8_t as8 = asflt;
            col->r = as8;
            col->g = as8;
            col->b = as8;
            col->a = 255;
        }

        Image img = { 0 };
        img.height = 128;
        img.width = 128;
        img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        img.mipmaps = 0;
        img.data = imgData;

        ExportImage(img, TextFormat("corrupted/%#lx.png", chunk.signature));
    }

    long current = ftell(f);
    printf("Ended at address %#lx.\n", current);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, current, SEEK_SET);

    void *lzCompData = malloc(size - current);
    fread(lzCompData, 1, size - current, f);

    //SaveFileData("corrupted/extra.Z", lzCompData, size - current);

}
