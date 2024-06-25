#include "filetypes/rw4.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct RWHeader {
    char magic[28];
    uint32_t type;
    uint32_t unknown1;
    uint32_t sectionCount;
    uint32_t unknown2[2];
    uint32_t sectionInfoOffset;
    uint32_t unknown3[4];
    uint32_t bufferDataOffset;
    uint32_t unknown4[7];
    uint32_t unknownBits;
    uint32_t unknown5[12];

    // Section Manifest
    uint32_t typeCode;
    uint32_t field0;
    uint32_t field1;
    uint32_t offset1;
    uint32_t offset2;
    uint32_t offset3;
    uint32_t offset4;
} RWHeader;

typedef struct RWSectionInfo {
    uint32_t dataOffset;
    uint32_t field4;
    uint32_t size;
    uint32_t alignment;
    uint32_t typeCodeIndex;
    uint32_t typeCode;
} RWSectionInfo;

RW4Data LoadRW4Data(unsigned char *data, int dataSize)
{
    RW4Data rw4data = { 0 };

    unsigned char *initData = data;

    printf("RW4 Info:\n");

    RWHeader header = { 0 };
    memcpy(&header, data, sizeof(RWHeader));
    data += sizeof(RWHeader);

    printf("Type: %d\n", header.type);
    printf("Section Count: %d\n", header.sectionCount);
    printf("Section Info Offset: %d\n", header.sectionInfoOffset);
    printf("Buffer Data Offset: %d\n", header.bufferDataOffset);
    
    printf("Section manifest:\n");
    printf("Offset 1: %d\n", header.offset1);
    printf("Offset 2: %d\n", header.offset2);
    printf("Offset 3: %d\n", header.offset3);
    printf("Offset 4: %d\n", header.offset4);

    RWSectionInfo *sectionInfos = malloc(sizeof(RWSectionInfo) * header.sectionCount);

    printf("Section info:\n");
    data = initData + header.sectionInfoOffset;
    for (int i = 0; i < header.sectionCount; i++)
    {
        RWSectionInfo sectionInfo = { 0 };
        memcpy(&sectionInfo, data, sizeof(RWSectionInfo));
        data += sizeof(RWSectionInfo);

        printf("\nSection %d:\n", i);
        printf("Data Offset: %d\n", sectionInfo.dataOffset);
        printf("Size: %d\n", sectionInfo.size);
        printf("Alignment: %d\n", sectionInfo.alignment);
        printf("Type Code Index: %d\n", sectionInfo.typeCodeIndex);
        printf("Type Code: %#x\n", sectionInfo.typeCode);

        sectionInfos[i] = sectionInfo;
    }

    for (int i = 0; i < header.sectionCount; i++) {
        RWSectionInfo sectionInfo = sectionInfos[i];
        data = initData + sectionInfo.dataOffset;

        switch (sectionInfo.typeCode)
        {
            default:
            {
                printf("Unrecognized type code %#x.\n");
                rw4data.corrupted = true;
                return rw4data;
            } break;
        }
    }

    return rw4data;
}
