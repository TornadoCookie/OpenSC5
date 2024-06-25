#include "filetypes/rw4.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <cpl_endian.h>

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

typedef struct RWKeyframeAnim {
    uint32_t channelNameOffset;
    uint32_t channelCount;
    uint32_t skeletonId;
    uint32_t fieldC;
    uint32_t channelDataOffset;
    uint32_t paddingEndOffset;
    uint32_t channelCount2;
    uint32_t field1c;
    float length;
    uint32_t field24;
    uint32_t flags;
    uint32_t channelInfoOffset;
} RWKeyframeAnim;

typedef struct RWSkeleton {
    uint32_t boneFlagOffset;
    uint32_t boneParentOffset;
    uint32_t boneNameOffset;
    uint32_t count;
    uint32_t skeletonId;
    uint32_t unknown1;
} RWSkeleton;

typedef struct RWSkeletonsInK {
    uint32_t object1;
    uint32_t pFunction;
    uint32_t arrayOffset;
    uint32_t object2;
    uint32_t skeleton;
    uint32_t arrayCount;
} RWSkeletonsInK;

typedef struct RWBBox {
    Vector3 min;
    uint32_t fieldc;
    Vector3 max;
    uint32_t field1c;
} RWBBox;

RW4Data LoadRW4Data(unsigned char *data, int dataSize)
{
    RW4Data rw4data = { 0 };

    unsigned char *initData = data;

    printf("RW4 Info:\n");

    RWHeader header = { 0 };
    memcpy(&header, data, sizeof(RWHeader));
    data += sizeof(RWHeader);

    printf("Type: %#x\n", header.type);
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

        printf("\nSection %d:\n", i);

        switch (sectionInfo.typeCode)
        {
            case 0x70001: // KeyframeAnim
            {
                RWKeyframeAnim keyframeAnim;
                memcpy(&keyframeAnim, data, sizeof(RWKeyframeAnim));

                printf("Keyframe Anim Info:\n");
                printf("Channel name offset: %d\n", keyframeAnim.channelNameOffset);
                printf("Channel count: %d\n", keyframeAnim.channelCount);
                printf("Skeleton Id: %#x\n", keyframeAnim.skeletonId);
                printf("Channel Data offset: %d\n", keyframeAnim.channelDataOffset);
                printf("Padding end offset: %d\n", keyframeAnim.paddingEndOffset);
                printf("Length: %f\n", keyframeAnim.length);
                printf("Flags: %#x\n", keyframeAnim.flags);
                printf("Channel Info Offset: %d\n", keyframeAnim.channelInfoOffset);
                printf("TODO\n");
            } break;
            case 0xff0001: // Animations
            {
                data += sizeof(uint32_t);
                uint32_t count = *(uint32_t*)data;
                data += sizeof(uint32_t);

                printf("Animations Info:\n");
                printf("Count: %d\n", count);
                for (int i = 0; i < count; i++)
                {
                    uint32_t animationIndex = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    uint32_t section = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    printf("Animation %#x: [Section %d]\n", animationIndex, section);
                }
            } break;
            case 0x70002: // Skeleton
            {
                RWSkeleton skeleton;
                memcpy(&skeleton, data, sizeof(RWSkeleton));
                data += sizeof(RWSkeleton);

                printf("Skeleton Info:\n");
                printf("Bone Flag Offset: %d\n", skeleton.boneFlagOffset);
                printf("Bone Parent Offset: %d\n", skeleton.boneParentOffset);
                printf("Bone Name Offset: %d\n", skeleton.boneNameOffset);
                printf("Bone Count: %d\n", skeleton.count);
                printf("Skeleton Id: %#x\n", skeleton.skeletonId);
                printf("TODO\n");
            } break;
            case 0x7000b: // SkeletonsInK
            {
                RWSkeletonsInK skeletonsink;
                memcpy(&skeletonsink, data, sizeof(RWSkeletonsInK));
                data += sizeof(RWSkeletonsInK);

                printf("SkeletonsInK Info:\n");
                printf("Array Offset: %d\n", skeletonsink.arrayOffset);
                printf("Skeleton: [Section %d]\n", skeletonsink.skeleton);
                printf("Count: %d\n", skeletonsink.arrayCount);

                printf("TODO\n");

            } break;
            case 0x80005: // BBox
            {
                RWBBox bbox;
                memcpy(&bbox, data, sizeof(RWBBox));
                data += sizeof(RWBBox);

                printf("Bounding Box Information:\n");
                printf("Min: {%f, %f, %f}\n", bbox.min.x, bbox.min.y, bbox.min.z);
                printf("Max: {%f, %f, %f}\n", bbox.max.x, bbox.max.y, bbox.max.z);
            } break;
            default:
            {
                printf("Unrecognized type code %#x.\n", sectionInfo.typeCode);
            } break;
        }
    }

    return rw4data;
}
