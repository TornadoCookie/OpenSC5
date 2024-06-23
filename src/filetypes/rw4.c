#include "filetypes/rw4.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static const char signature[] = { 137, 82, 87, 52, 119, 51, 50, 0, 13, 10, 26, 10, 0, 32, 4, 0, 52, 53, 52, 0, 48, 48, 48, 0, 0, 0, 0, 0 };

typedef struct RW4Header {
    char signature[28];
    uint32_t fileTypeCode;
    uint32_t sectionCount;
    uint32_t sectionCount2;
    uint32_t fileTypeConstant;
    uint32_t null;
    uint32_t sectionIndexBegin;
    uint32_t firstHeaderSectionBegin;
    uint32_t unknown1[3];
    uint32_t sectionIndexEnd;
    uint32_t fileTypeConstant2;
    uint32_t fileSize;
    uint32_t unknown2[5];
    uint32_t unknown;
    uint32_t unknown3[5];
    uint32_t unknown4[7];
} RW4Header;

typedef struct RW4File {
    RW4Header header;
} RW4File;

RW4Data LoadRW4Data(unsigned char *data, int dataSize)
{
    RW4Data rw4data = { 0 };
    RW4File file = { 0 };

    unsigned char *initData = data;

    printf("RW4 Data:\n");

    memcpy(&file.header, data, sizeof(RW4Header));
    data += sizeof(RW4Header);

    file.header.fileSize += file.header.sectionIndexEnd;

    data = initData + file.header.firstHeaderSectionBegin;
    data += sizeof(uint32_t);

    uint32_t offsets[6] = { 0 };
    memcpy(offsets, data, sizeof(uint32_t)*6);
    data += sizeof(uint32_t)*6;

    for (int i = 0; i < 6; i++)
    {
        offsets[i] += file.header.firstHeaderSectionBegin;
    }

    data += sizeof(uint32_t);
    uint32_t sectionTypeCount = *(uint32_t*)data;
    data += sizeof(uint32_t);
    data += sizeof(uint32_t);

    uint32_t *sectionTypes = malloc(sectionTypeCount * sizeof(uint32_t));
    memcpy(sectionTypes, data, sectionTypeCount * sizeof(uint32_t));
    data += sectionTypeCount * sizeof(uint32_t);

    data += sizeof(uint32_t);

    return rw4data;
}