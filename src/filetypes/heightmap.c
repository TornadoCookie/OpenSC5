#include "filetypes/heightmap.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cpl_endian.h>

typedef struct __attribute__ ((packed)) HeightmapHeader {
    char unknown[7];
    uint32_t height; // may be width
    uint32_t width;  // may be height
    uint16_t unknown2;
    uint16_t unknown3;
} HeightmapHeader;

Color eight2col(uint8_t d)
{
    return (Color){d, d, d, 255};
}

HeightmapData LoadHeightmapData(unsigned char *data, int dataSize)
{
    HeightmapData heightmapData = { 0 };
    
    HeightmapHeader *header = (HeightmapHeader *)data;
    data += 0x14;

    Color *imgDatas[4];

    for (int i = 0; i < 4; i++)
    {
        imgDatas[i] = malloc(0x4000 * sizeof(Color));
    }


    uint32_t *data32 = (uint32_t *)data;
    TRACELOG(LOG_DEBUG, "w %d h %d\n", header->height, header->width);

    for (int i = 0; i < 0x4000; i++)
    {
        uint32_t asint = data32[i];
        TRACELOGNONL(LOG_DEBUG, "%08x ", asint);
        uint8_t *as8s = (uint8_t *)&asint;

        imgDatas[0][i] = eight2col(as8s[0]);
        imgDatas[1][i] = eight2col(as8s[1]);
        imgDatas[2][i] = eight2col(as8s[2]);
        imgDatas[3][i] = eight2col(as8s[3]);
    }

    Image imgs[4];

    for (int i = 0; i < 4; i++)
    {
        imgs[i].width = header->width;
        imgs[i].height = header->height;
        imgs[i].format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        imgs[i].data = imgDatas[i];
        imgs[i].mipmaps = 1;
        ExportImage(imgs[i], TextFormat("heightmap_%d.png", i));
    }

    

    heightmapData.heightmap = imgs[0];
    heightmapData.corrupted = true;

    return heightmapData;
}
