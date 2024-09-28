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

StateFile LoadStateFile(const char *filename)
{
    FILE *f = fopen(filename, "rb");

    fseek(f, 0x2C, SEEK_SET); // skip the header, it is giving us no useful info at the moment.

    // TODO this assumes the file is already gzip decompressed.
    while (!feof(f))
    {
        StateFileChunk chunk = { 0 };

        fread(&chunk, sizeof(StateFileChunk), 1, f);

        //if (chunk.null != 0) printf("expected NULL, got %#x\n", chunk.null);
        printf("signature: %#lx\n", chunk.signature);

        chunk.size = htobe32(chunk.size);

        printf("size: %d\n", chunk.size);
        
        uint32_t *imgData = malloc(chunk.size * 4);
        fread(imgData, 4, chunk.size, f);

        for (int i = 0; i < chunk.size; i++)
        {
            Color *col = ((Color*)imgData)+i;
            uint32_t asint = imgData[i];
            asint = htobe32(asint);
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


}
