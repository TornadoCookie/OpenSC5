#include "filetypes/package.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct PackageHeader {
    char magic[4];              //00
    uint32_t majorVersion;      //04
    uint32_t minorVersion;      //08
    uint32_t unknown[3];        //0C
    uint32_t dateCreated;       //18
    uint32_t dateModified;      //1C
    uint32_t indexMajorVersion; //20
    uint32_t indexEntryCount;   //24
    uint32_t firstIndexEntryOffset; //28
    uint32_t indexSize;         //2C
    uint32_t holeEntryCount;    //30
    uint32_t holeOffset;        //34
    uint32_t holeSize;          //38
    uint32_t indexMinorVersion; //3C
    uint32_t indexOffset;       //40
    uint32_t unknown2;          //44
    unsigned char reserved[24]; //48
                                //5C
} PackageHeader;

typedef struct IndexEntry {
    uint32_t type;
    uint32_t group;
    uint32_t instance;
    uint32_t chunkOffset;
    uint32_t diskSize;
    uint32_t memSize;
    uint16_t compressed;
    uint16_t unknown;
    
    bool isCompressed;
} IndexEntry;

typedef struct IndexData {
    uint32_t null;
} IndexData;

typedef struct Index {
    uint32_t indexType;
    
    IndexData data;
} Index;

static void readuint(uint32_t *ret, FILE *f)
{
    fread(ret, sizeof(uint32_t), 1, f);

    if (feof(f))
    {
        printf("Unexpected end of file: %lu.\n", ftell(f));
        exit(EXIT_FAILURE);
    }
}

static uint32_t little2big32(uint32_t le)
{
    return __builtin_bswap32(le);
}

static uint16_t little2big16(uint16_t le)
{
    return __builtin_bswap16(le);
}

static void ProcessPackageData(unsigned char *data, int dataSize, uint32_t dataType)
{
    switch (dataType)
    {
        case 0x00B1B104: // Properties files https://simswiki.info/wiki.php?title=Spore:00B1B104
        {
            uint32_t variableCount = little2big32(*(uint32_t*)data);

            data += sizeof(uint32_t);

            printf("Properties Info:\n");
            printf("Variable count: %d\n", variableCount);
            
            for (int i = 0; i < variableCount; i++)
            {
                printf("\nVariable %d:\n", i);

                uint32_t identifier = little2big32(*(uint32_t*)data);
                data += sizeof(uint32_t);
                uint16_t type = little2big16(*(uint16_t*)data);
                data += sizeof(uint16_t);
                uint16_t specifier = little2big16(*(uint16_t*)data);
                data += sizeof(uint16_t);

                printf("Identifier: %#x\n", identifier);
                printf("Type: %#x\n", type);
                printf("Specifier: %#x\n", specifier);

                uint32_t arrayNumber = 1;
                uint32_t arraySize = 0;
                bool isArray = false;

                if (((specifier & 0x30) != 0) && ((specifier & 0x40) == 0))
                {
                    isArray = true;
                    arrayNumber = little2big32(*(uint32_t*)data);
                    data += sizeof(uint32_t);
                    arraySize = little2big32(*(uint32_t*)data);
                    data += sizeof(uint32_t);

                    printf("Array nmemb: %d\n", arrayNumber);
                    printf("Array item size: %u\n", arraySize);
                }

                for (int j = 0; j < arrayNumber; j++)
                {
                    switch (type)
                    {
                        case 0x20: // key type
                        {
                            uint32_t file = *(uint32_t*)data;
                            data += sizeof(uint32_t);
                            uint32_t type = *(uint32_t*)data;
                            data += sizeof(uint32_t);
                            uint32_t group = *(uint32_t*)data;
                            data += sizeof(uint32_t);

                            if (!isArray)
                            {
                                data += sizeof(uint32_t);
                            }

                            printf("File: %#x\n", file);
                            printf("Type: %#x\n", type);
                            printf("Group: %#x\n", group);
                        } break;
                        case 9: // int32 type
                        {
                            int32_t value = little2big32(*(int32_t*)data);
                            data += sizeof(int32_t);

                            printf("Value: %#x\n", value);
                        } break;
                        case 0x32: // colorRGB type
                        {
                            float r = *(float*)data;
                            data += sizeof(float);
                            float g = *(float*)data;
                            data += sizeof(float);
                            float b = *(float*)data;
                            data += sizeof(float);

                            if (!isArray)
                            {
                                data += sizeof(uint32_t);
                            }

                            printf("Value: {%f, %f, %f}\n", r, g, b);
                        } break;
                        default:
                        {
                            printf("Unrecognized variable type.\n");
                            return;
                        } break;
                    }
                }
            }
        } break;
        default:
        {
            printf("Unknown data type %#X.\n", dataType);
        } break;
    }
}

static unsigned char *DecompressDBPF(unsigned char *data, int dataSize, int outDataSize)
{
    unsigned char *ret = malloc(outDataSize);
    unsigned char *initData = data;

    uint8_t compressionType = data[0];
    uint32_t uncompressedSize;

    printf("Compression Type: %#x\n", compressionType);

    if (compressionType != 0x10)
    {
        printf("Unrecognized compression type.\n");
        free(ret);
        return NULL;
    }

    memcpy(&uncompressedSize, data + 2, 3);
    //printf("Uncompressed Size: %d\n", uncompressedSize);

    data += 5;

    unsigned char *retCursor = ret;

    while (initData - data <= dataSize)
    {
        uint8_t byte0 = *data;
        data++;
        int numPlainText = 0;
        int numToCopy = 0;
        int copyOffset = 0;
        //printf("Control character: %#x\n", byte0);
        if (byte0 >= 0xE0 && byte0 <= 0xFB)
        {
            numPlainText = ((byte0 & 0x1F) << 2) + 4;
        }
        else if (byte0 >= 0x00 && byte0 <= 0x7F)
        {
            uint8_t byte1 = *data;
            data++;
            numPlainText = byte0 & 0x03;
            numToCopy = ((byte0 & 0x1C) >> 2) + 3;
            copyOffset = ((byte0 & 0x60) << 3) + byte1 + 1;
        }
        else if (byte0 >= 0x80 && byte0 <= 0xBF)
        {
            uint8_t byte1 = *data;
            data++;
            uint8_t byte2 = *data;
            data++;

            numPlainText = ((byte1 & 0xC0) >> 6) & 0x03;
            numToCopy = (byte0 & 0x3F) + 4;
            copyOffset = ((byte1 & 0x3F) << 8) + byte2 + 1;
        }
        else if (byte0 >= 0xFC & byte0 <= 0xFF)
        {
            numPlainText = byte0 & 0x03;
            numToCopy = 0; 
        }
        else if (byte0 >= 0xC0 && byte0 <= 0xDF)
        {
            uint8_t byte1 = *data;
            data++;
            uint8_t byte2 = *data;
            data++;
            uint8_t byte3 = *data;
            data++;

            numPlainText = byte0 & 0x03;
            numToCopy = ((byte0 & 0x0C) << 6) + byte3 + 5;
            copyOffset = ((byte0 & 0x10) << 12) + (byte1 << 8) + byte2 + 1;
        }
        else
        {
            printf("Unrecognized control character %#x.\n", byte0);
            return NULL;
        }

        memcpy(retCursor, data, numPlainText);
        retCursor += numPlainText;
        data += numPlainText;

        memcpy(retCursor, retCursor - copyOffset - 1, numToCopy);
        retCursor += numToCopy;

        if (byte0 >= 0xFC & byte0 <= 0xFF) break;
    }

    return ret;
}

void LoadPackageFile(FILE *f)
{
    PackageHeader header;
    fread(&header, sizeof(PackageHeader), 1, f);

    printf("Header:\n");
    printf("Magic: %.4s\n", header.magic);
    printf("Major Version #: %d\n", header.majorVersion);
    printf("Minor Version #: %d\n", header.minorVersion);
    printf("Index Entry Count: %d\n", header.indexEntryCount);
    printf("Index Size: %d\n", header.indexSize);
    printf("Index Major Version: %d\n", header.indexMajorVersion);
    printf("Index Minor Version: %d\n", header.indexMinorVersion);
    printf("Index Offset: %d\n", header.indexOffset);

    fseek(f, header.indexOffset, SEEK_SET);
    Index index = { 0 };

    readuint(&index.indexType, f);

    printf("\nIndex information:\n");
    printf("Index Type: %d\n", index.indexType);

    IndexData indexData = index.data;

    //readuint(&indexData.null, f);

    uint32_t indexTypeId = 0xCAFEBABE;
    if ((index.indexType & (1 << 0)) == 1 << 0)
    {
        readuint(&indexTypeId, f);
    }

    uint32_t indexGroupContainer = 0xCAFEBABE;
    if ((index.indexType & (1 << 1)) == 1 << 1)
    {
        readuint(&indexGroupContainer, f);
    }

    uint32_t indexUnknown = 0xCAFEBABE;
    if ((index.indexType & (1 << 2)) == 1 << 2)
    {
        readuint(&indexUnknown, f);
    }

    IndexEntry *entries = malloc(sizeof(IndexEntry) * header.indexEntryCount);

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        IndexEntry entry = { 0 };

        if ((index.indexType & (1 << 0)) == 1 << 0)
        {
            entry.type = indexTypeId;
        }
        else
        {
            readuint(&entry.type, f);
        }

        if ((index.indexType & (1 << 1)) == 1 << 1)
        {
            entry.group = indexGroupContainer;
        }
        else
        {
            readuint(&entry.group, f);
        }

        if ((index.indexType & (1 << 2)) == 1 << 2)
        {
            uint32_t unk = indexUnknown;
        }
        else
        {
            uint32_t unk;
            readuint(&unk, f);
        }

        readuint(&entry.instance, f);

        readuint(&entry.chunkOffset, f);

        readuint(&entry.diskSize, f);
        entry.diskSize &= ~0x80000000;
        readuint(&entry.memSize, f);
        fread(&entry.compressed, sizeof(uint16_t), 1, f);
        fread(&entry.unknown, sizeof(uint16_t), 1, f);

        entry.isCompressed = (entry.compressed == 0xFFFF);

        printf("\nEntry %d:\n", i);
        printf("Type: %#X\n", entry.type);
        printf("Group: %u\n", entry.group);
        printf("Instance: %u\n", entry.instance);
        printf("Chunk Offset: %u\n", entry.chunkOffset);
        printf("Disk Size: %u\n", entry.diskSize);
        printf("Mem Size: %u\n", entry.memSize);
        printf("Compressed? %s\n", entry.isCompressed?"yes":"no");

        entries[i] = entry;
    }

    printf("\nData Cycle.\n");

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        IndexEntry entry = entries[i];

        printf("\nEntry %d:\n", i);

        if (fseek(f, entry.chunkOffset, SEEK_SET) == -1)
        {
            perror("Unexpected error occurred");
        }

        unsigned char *data = malloc(entry.diskSize);

        fread(data, 1, entry.diskSize, f);

        if (feof(f))
        {
            printf("Unexpected end of file.\n");
        }

        if (entry.isCompressed)
        {
            unsigned char *uncompressed = DecompressDBPF(data, entry.diskSize, entry.memSize);
            if (uncompressed)
            {
                ProcessPackageData(uncompressed, entry.memSize, entry.type);
                for (int i = 0; i < entry.memSize; i++)
                {
                    printf("%#x ", uncompressed[i]);
                }
                puts("");
                free(uncompressed);
            }
        }
        else
        {
            ProcessPackageData(data, entry.diskSize, entry.type);
            for (int i = 0; i < entry.diskSize; i++)
            {
                printf("%#x ", data[i]);
            }
            puts("");
        }

        free(data);
    }
}
