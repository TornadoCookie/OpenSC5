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

#define INDEX_HAS_RESOURCETYPE  0b00000001
#define INDEX_HAS_RESOURCEGROUP 0b00000010
#define INDEX_HAS_INSTANCEHI    0b00000100
#define INDEX_HAS_INSTANCELO    0b00001000
#define INDEX_HAS_CHUNKOFFSET   0b00010000
#define INDEX_HAS_FILESIZE      0b00100000
#define INDEX_HAS_MEMSIZE       0b01000000
#define INDEX_HAS_COMPRESSED    0b10000000

typedef struct IndexEntry4 {
    uint32_t type;
    uint32_t group;
    uint32_t instance;
    uint32_t chunkOffset;
    uint32_t diskSize;
    uint32_t memSize;
    uint16_t compressed;
    uint16_t unknown;
} IndexEntry4;

typedef struct IndexData4 {
    uint32_t null;
    IndexEntry4 *entries;
} IndexData4;

typedef struct Index {
    uint32_t indexType;
    
    union {
        IndexData4 type4;
    } data;
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

    switch (index->indexType)
    {
        case 4:
        {
            IndexData4 indexData = index->data.type4;
            data += sizeof(uint32_t); // indexType
            data += sizeof(uint32_t); // null

            for (int i = 0; i < header.indexEntryCount; i++)
            {
                IndexEntry4 *entry = (IndexEntry4*)data;
                printf("\nEntry %d:\n", i);
                printf("Type: %u\n", entry->type);
                printf("Group: %u\n", entry->group);
                printf("Instance: %u\n", entry->instance);
                printf("Chunk Offset: %u\n", entry->chunkOffset);
                printf("Disk Size: %u\n", entry->diskSize);
                printf("Mem Size: %u\n", entry->memSize);
                printf("Compressed? %s\n", entry->compressed == 0xFFFF?"yes":"no");

                data += sizeof(IndexEntry4);
            }
        } break;
        default:
        {
            printf("Unknown index type %d.\n", index->indexType);
        } break;
    }
}
