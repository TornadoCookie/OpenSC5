#include "filetypes/rast.h"
#include <stdint.h>
#include <cpl_endian.h>
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

    unsigned char *initData = data;

    memcpy(&file.header, data, sizeof(RasterFileHeader));
    data += sizeof(RasterFileHeader);

    file.header.type = htobe32(file.header.type);
    file.header.width = htobe32(file.header.width);
    file.header.height = htobe32(file.header.height);
    file.header.mipmapct = (htobe32(file.header.mipmapct) & 0xFF) >> 1;
    file.header.pixelwidth = htobe32(file.header.pixelwidth) & 0xFF;
    file.header.pixelformat = htobe32(file.header.pixelformat) & 0xFF;

    printf("Type: %d\n", file.header.type);
    printf("Width: %d\n", file.header.width);
    printf("Height: %d\n", file.header.height);
    printf("Mipmap Count: %d\n", file.header.mipmapct);
    printf("PixelWidth: %d\n", file.header.pixelwidth);
    printf("PixelFormat: %#x\n", file.header.pixelformat);

    if (file.header.pixelwidth != 8)
    {
        printf("I don't know how to deal with any other pixelwidth other than 8.\n");
        rastData.corrupted = true;
        return rastData;
    }

    if (file.header.pixelformat != 0x15)
    {
        printf("I don't know how to deal with any other pixelformat other than 0x15.\n");
        rastData.corrupted = true;
        return rastData;
    }

    file.images = malloc(sizeof(RasterFileImage) * file.header.mipmapct);
    Image img = { 0 };

    img.width = file.header.width;
    img.height = file.header.height;
    img.mipmaps = 1;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    Color *imgData = malloc(file.header.width*file.header.height*sizeof(Color));

    for (int i = 0; i < 1; i++)
    {
        RasterFileImage rastImg = { 0 };

        if (4*file.header.width*file.header.height > dataSize)
        {
            printf("{Corruption Detected.}\n");
            rastData.corrupted = true;
            return rastData;
        }

        rastImg.blocksize = htobe32(*(uint32_t*)data);
        data += sizeof(uint32_t);

        printf("Image %d: Blocksize %d\n", i, rastImg.blocksize);

        for (int j = 0; j < file.header.width*file.header.height; j++)
        {
            uint8_t b = *(uint8_t*)data;
            data++;
            uint8_t g = *(uint8_t*)data;
            data++;
            uint8_t r = *(uint8_t*)data;
            data++;
            uint8_t a = *(uint8_t*)data;
            data++;

            imgData[j] = (Color){r, g, b, a};

            //printf("%d/%d: %d, %d, %d, %d.\n", i*4, rastImg.blocksize, r, g, b, a);
        }
    }

    img.data = imgData;
    printf("Loaded.\n");

    rastData.img = img;

    return rastData;
}
