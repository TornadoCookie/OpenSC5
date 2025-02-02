#include "filetypes/bnk.h"
#include "filetypes/wwriff.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cpl_raylib.h>

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

    ContentIndex *contentIndices = NULL;
    int contentIndexCount;

    while ((data - initData) < dataSize)
    {
        char *signature = data;
        if (!strncmp(signature, "BKHD", 4))
        {
            BnkHeader *header = (BnkHeader*)data;
            bnkData.pointsTo = header->pointsTo;

            TRACELOG(LOG_DEBUG, "Version: %d\n", header->version);

            data += header->size + 8;
        }
        else if (!strncmp(signature, "HIRC", 4))
        {
            data += 4;
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
                        TRACELOG(LOG_DEBUG, "Unrecognized HIRC type %#x.\n", type);
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

            TRACELOG(LOG_DEBUG, "Content Index Count: %d\n", contentIndexCount);

            for (int i = 0; i < contentIndexCount; i++)
            {
                TRACELOG(LOG_DEBUG, "Index %d:\n", i);
                TRACELOG(LOG_DEBUG, "Id: %#x\n", contentIndices[i].id);
                TRACELOG(LOG_DEBUG, "Offset: %d\n", contentIndices[i].offset);
                TRACELOG(LOG_DEBUG, "Size: %d\n", contentIndices[i].size);
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
            TRACELOG(LOG_WARNING, "Unrecognized signature %.4s\n", signature);
            bnkData.corrupted = true;
            return bnkData;
        }
    }

    if (contentIndices == NULL)
    {
        TRACELOG(LOG_ERROR, "Error: BNK has no content indices.\n");
        bnkData.corrupted = true;
        return bnkData;
    }

    bnkData.waveCount = contentIndexCount;
    bnkData.waves = malloc(bnkData.waveCount * sizeof(Wave));

    for (int i = 0; i < contentIndexCount; i++)
    {
        ContentIndex index = contentIndices[i];

        data = soundDataOffset + index.offset;

        {
            TRACELOG(LOG_INFO, "BNK: Loading %#X (%d/%d)", index.id, i, contentIndexCount);
            
            bnkData.waves[i] = LoadWWRiffWave(data, index.size);
        }
    }

    return bnkData;
}
