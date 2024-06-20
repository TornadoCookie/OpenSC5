#include "filetypes/package.h"
#include <stdint.h>

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

void LoadPackageFile(unsigned char *data)
{
    PackageHeader header = *(PackageHeader*)data;

    printf("Header:\n");
    printf("Magic: %.4s\n", header.magic);
    printf("Major Version #: %d\n", header.majorVersion);
    printf("Minor Version #: %d\n", header.minorVersion);
    printf("Index Entry Count: %d\n", header.indexEntryCount);
    printf("Index Size: %d\n", header.indexSize);
    printf("Index Major Version: %d\n", header.indexMajorVersion);
    printf("Index Minor Version: %d\n", header.indexMinorVersion);
    printf("Index Offset: %d\n", header.indexOffset);

    Index *index = (Index*)(data + header.indexOffset);

    printf("\nIndex information:\n");
    printf("Index Type: %d\n", index->indexType);

    IndexData indexData = index->data;

    uint32_t indexTypeId = 0xCAFEBABE;
    if ((index->indexType & (1 << 0)) == 1 << 0)
    {
        indexTypeId = *(uint32_t*)data;
        data += sizeof(uint32_t);
    }

    uint32_t indexGroupContainer = 0xCAFEBABE;
    if ((index->indexType & (1 << 1)) == 1 << 1)
    {
        indexGroupContainer = *(uint32_t*)data;
        data += sizeof(uint32_t);
    }

    uint32_t indexUnknown = 0xCAFEBABE;
    if ((index->indexType & (1 << 2)) == 1 << 2)
    {
        indexUnknown = *(uint32_t*)data;
        data += sizeof(uint32_t);
    }

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        IndexEntry *entry = (IndexEntry*)data;

        if ((index->indexType & (1 << 0)) == 1 << 0)
        {
            entry->type = indexTypeId;
        }
        else
        {
            entry->type = *(uint32_t*)data;
            data += sizeof(uint32_t);
        }

        if ((index->indexType & (1 << 1)) == 1 << 1)
        {
            entry->group = indexGroupContainer;
        }
        else
        {
            entry->group = *(uint32_t*)data;
            data += sizeof(uint32_t);
        }

        if ((index->indexType & (1 << 2)) == 1 << 2)
        {
            uint32_t unk = indexUnknown;
        }
        else
        {
            uint32_t unk = *(uint32_t*)data;
            data += sizeof(uint32_t);
        }

        entry->instance = *(uint32_t*)data;
        data += sizeof(uint32_t);

        entry->diskSize = (*(uint32_t*)data) & ~0x80000000;
        data += sizeof(uint32_t);

        entry->memSize = *(uint32_t*)data;
        data += sizeof(uint32_t);

        entry->compressed = *(uint16_t*)data;
        data += sizeof(uint16_t);

        entry->unknown = *(uint16_t*)data;
        data += sizeof(uint16_t);

        printf("\nEntry %d:\n", i);
        printf("Type: %#X\n", entry->type);
        printf("Group: %u\n", entry->group);
        printf("Instance: %u\n", entry->instance);
        printf("Chunk Offset: %u\n", entry->chunkOffset);
        printf("Disk Size: %u\n", entry->diskSize);
        printf("Mem Size: %u\n", entry->memSize);
        printf("Compressed? %s\n", entry->compressed == 0xFFFF?"yes":"no");
    }
}
