#include "filetypes/rast.h"
#include <stdint.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct RasterFileHeader {
    uint32_t type;
    uint32_t width;
    uint32_t height;
    uint32_t mipmapct;
    uint32_t pixelwidth;
    uint32_t pixelformat;
} RasterFileHeader;

typedef struct RasterFileImage {
    uint32_t blocksize;
    char *data;
} RasterFileImage;

typedef struct RasterFile {
    RasterFileHeader header;
    RasterFileImage *images;
} RasterFile;

RastData LoadRastData(unsigned char *data, int dataSize)
{
    RastData rastData = { 0 };
    printf("Raster info:\n");
    RasterFile file = { 0 };

    memcpy(&file.header, data, sizeof(RasterFileHeader));
    data += sizeof(RasterFileHeader);

    file.header.type = htobe32(file.header.type);
    file.header.width = htobe32(file.header.width);
    file.header.height = htobe32(file.header.height);
    file.header.mipmapct = htobe32(file.header.mipmapct);
    file.header.pixelwidth = htobe32(file.header.pixelwidth);
    file.header.pixelformat = htobe32(file.header.pixelformat);

    printf("Type: %d\n", file.header.type);
    printf("Width: %d\n", file.header.width);
    printf("Height: %d\n", file.header.height);
    printf("Mipmap Count: %d\n", file.header.mipmapct);
    printf("PixelWidth: %d\n", file.header.pixelwidth);
    printf("PixelFormat: %#x\n", file.header.pixelformat);

    rastData.corrupted = true;
    return rastData;

    file.images = malloc(sizeof(RasterFileImage) * file.header.mipmapct);

    for (int i = 0; i < file.header.mipmapct; i++)
    {
        RasterFileImage img = { 0 };

        img.blocksize = *(uint32_t*)data;
        data += sizeof(uint32_t);

        printf("Image %d: Blocksize %d\n", i, img.blocksize);
    }

    return rastData;
}
