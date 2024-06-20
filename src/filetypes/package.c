#include "filetypes/package.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct PackageHeader {
    char magic[4];
    uint32_t majorVersion;
    uint32_t minorVersion;
    uint32_t unknown[3];
    uint32_t dateCreated;
    uint32_t dateModified;
    uint32_t indexMajorVersion;
    uint32_t indexEntryCount;
    uint32_t firstIndexEntryOffset;
    uint32_t indexSize;
    uint32_t holeEntryCount;
    uint32_t holeOffset;
    uint32_t holeSize; 
    uint32_t indexMinorVersion;
    uint32_t indexOffset;
    uint32_t unknown2;
    unsigned char reserved[24];
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
} IndexEntry;

typedef struct IndexData {
    uint32_t null;
    IndexEntry *entries;
} IndexData;

typedef struct Index {
    uint32_t indexType;
    
    IndexData data;
} Index;

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
    Index index;

    fread(&index, sizeof(Index), 1, f);

    printf("\nIndex information:\n");
    printf("Index Type: %d\n", index.indexType);

    IndexData indexData = index.data;

    uint32_t indexTypeId = 0xCAFEBABE;
    if ((index.indexType & (1 << 0)) == 1 << 0)
    {
        fread(&indexTypeId, sizeof(uint32_t), 1, f);
    }

    uint32_t indexGroupContainer = 0xCAFEBABE;
    if ((index.indexType & (1 << 1)) == 1 << 1)
    {
        fread(&indexGroupContainer, sizeof(uint32_t), 1, f);
    }

    uint32_t indexUnknown = 0xCAFEBABE;
    if ((index.indexType & (1 << 2)) == 1 << 2)
    {
        fread(&indexUnknown, sizeof(uint32_t), 1, f);
    }

    IndexEntry *entries = malloc(sizeof(IndexEntry) * header.indexEntryCount);

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        IndexEntry entry;

        if ((index.indexType & (1 << 0)) == 1 << 0)
        {
            entry.type = indexTypeId;
        }
        else
        {
            fread(&entry.type, sizeof(uint32_t), 1, f);
        }

        if ((index.indexType & (1 << 1)) == 1 << 1)
        {
            entry.group = indexGroupContainer;
        }
        else
        {
            fread(&entry.group, sizeof(uint32_t), 1, f);
        }

        if ((index.indexType & (1 << 2)) == 1 << 2)
        {
            uint32_t unk = indexUnknown;
        }
        else
        {
            uint32_t unk;
            fread(&unk, sizeof(uint32_t), 1, f);
        }

        fread(&entry.instance, sizeof(uint32_t), 1, f);
        fread(&entry.diskSize, sizeof(uint32_t), 1, f);
        fread(&entry.memSize, sizeof(uint32_t), 1, f);
        fread(&entry.compressed, sizeof(uint16_t), 1, f);
        fread(&entry.unknown, sizeof(uint16_t), 1, f);

        printf("\nEntry %d:\n", i);
        printf("Type: %#X\n", entry.type);
        printf("Group: %u\n", entry.group);
        printf("Instance: %u\n", entry.instance);
        printf("Chunk Offset: %u\n", entry.chunkOffset);
        printf("Disk Size: %u\n", entry.diskSize);
        printf("Mem Size: %u\n", entry.memSize);
        printf("Compressed? %s\n", entry.compressed == 0xFFFF?"yes":"no");

        //unsigned char *entryData = data + entry->chunkOffset;
        printf("Data:\n");
        //printf("%#x %#x %#x\n", entryData[0], entryData[1], entryData[2]);

        entries[i] = entry;
    }
}
