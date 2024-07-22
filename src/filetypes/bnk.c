#include "filetypes/bnk.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwwriff.h>

typedef struct BnkHeader {
    char signature[4]; // BKHD
    uint32_t size;  // 0x18000000
    uint32_t version;  // 0x41000000
    uint32_t pointsTo; // Identifier for a file in the same group.
} BnkHeader;

typedef struct ContentIndex {
    uint32_t id;
    uint32_t offset;
    uint32_t size;
} ContentIndex;

// Based on bnkextr: https://github.com/eXpl0it3r/bnkextr
BnkData LoadBnkData(unsigned char *data, int dataSize)
{
    BnkData bnkData = { 0 };

    unsigned char *soundDataOffset;
    unsigned char *initData = data;

    ContentIndex *contentIndices;
    int contentIndexCount;

    while ((data - initData) < dataSize)
    {
        char *signature = data;
        if (!strncmp(signature, "BKHD", 4))
        {
            BnkHeader *header = (BnkHeader*)data;
            bnkData.pointsTo = header->pointsTo;

            printf("Version: %d\n", header->version);

            data += header->size + 8;
        }
        else if (!strncmp(signature, "HIRC", 4))
        {
            uint32_t objectCount = *(uint32_t*)data;
            data += sizeof(uint32_t);

            for (int i = 0; i < objectCount; i++)
            {
                data += 2;
                unsigned char type = *data;
                data++;

                uint32_t size = *(uint32_t*)data;
                data += sizeof(uint32_t);

                uint32_t id = *(uint32_t*)data;
                data += sizeof(uint32_t);

                switch (type)
                {
                    default:
                    {
                        printf("Unrecognized HIRC type %#x.\n", type);
                        bnkData.corrupted = true;
                        return bnkData;
                    } break;
                }
            }
        }
        else if (!strncmp(signature, "DIDX", 4))
        {
            data += 4;
            uint32_t size = *(uint32_t*)data;
            data += sizeof(uint32_t);

            contentIndexCount = size / sizeof(ContentIndex);
            contentIndices = malloc(size);

            memcpy(contentIndices, data, size);

            data += size;

            printf("Content Index Count: %d\n", contentIndexCount);

            for (int i = 0; i < contentIndexCount; i++)
            {
                printf("Index %d:\n", i);
                printf("Id: %#x\n", contentIndices[i].id);
                printf("Offset: %d\n", contentIndices[i].offset);
                printf("Size: %d\n", contentIndices[i].size);
            }
        }
        else if (!strncmp(signature, "DATA", 4))
        {
            data += 4;
            uint32_t size = *(uint32_t*)data;
            data += sizeof(uint32_t);

            soundDataOffset = data;

            data += size;
        }
        else
        {
            printf("Unrecognized signature %.4s\n", signature);
            bnkData.corrupted = true;
            return bnkData;
        }
    }

    bnkData.waveCount = contentIndexCount;
    bnkData.waves = malloc(bnkData.waveCount * sizeof(Wave));

    for (int i = 0; i < contentIndexCount; i++)
    {
        ContentIndex index = contentIndices[i];

        data = soundDataOffset + index.offset;

        {
            FILE *f = fopen(TextFormat("corrupted/BNK_%#X.wem", index.id), "wb");
            fwrite(data, 1, index.size, f);
            fclose(f);

            WWRiff *wwriff = WWRiff_Create(TextFormat("corrupted/BNK_%#X.wem", index.id), "packed_codebooks_aoTuV_603.bin", false, false, NO_FORCE_PACKET_FORMAT);
            WWRiff_GenerateOGG(wwriff, TextFormat("corrupted/BNK_%#X.ogg", index.id));
            remove(TextFormat("corrupted/BNK_%#X.wem", index.id));
        }
    }

    return bnkData;
}
