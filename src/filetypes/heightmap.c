#include "filetypes/heightmap.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cpl_endian.h>

typedef struct HeightmapHeader {
    uint32_t unknown1;
    uint16_t unknown2;
    uint16_t width;
    uint16_t unknown3;
    uint16_t height;
    uint32_t unknown4;
    uint32_t unknown5;
} HeightmapHeader;

HeightmapData LoadHeightmapData(unsigned char *data, int dataSize)
{
    HeightmapData heightmapData = { 0 };

    HeightmapHeader *header = data;
    data += 0x13;

    header->height = htobe32(header->height);
    header->width = htobe32(header->width);

    Color *imgData = malloc(0x20000 * sizeof(Color));

    for (int i = 0; i < 0x20000; i++)
    {
        uint8_t as8 = data[i];
        Color col = {
            as8, as8, as8, 255
        };
        imgData[i] = col;
    }

    Image img = { 0 };
    img.width = 256;
    img.height = 512;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    img.data = imgData;
    img.mipmaps = 1;

    heightmapData.heightmap = img;
    heightmapData.corrupted = true;

    return heightmapData;
}
