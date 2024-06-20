#include "filetypes/package.h"
#include <stdint.h>

typedef struct PackageHeader {
    char magic[4];
    uint32_t major;
    uint32_t minor;
    char unknown1[24];
    uint32_t indexEntryCount;
    char unknown2[4];
    uint32_t indexSize;
    char unknown3[12];
    uint32_t index_version;
    uint32_t indexPosition;
    char unknown4[28];
} PackageHeader;

#define INDEX_HAS_RESOURCETYPE  0b00000001
#define INDEX_HAS_RESOURCEGROUP 0b00000010
#define INDEX_HAS_INSTANCEHI    0b00000100
#define INDEX_HAS_INSTANCELO    0b00001000
#define INDEX_HAS_CHUNKOFFSET   0b00010000
#define INDEX_HAS_FILESIZE      0b00100000
#define INDEX_HAS_MEMSIZE       0b01000000
#define INDEX_HAS_COMPRESSED    0b10000000

typedef struct Index {
    uint32_t type;

    uint32_t ResourceType;
    uint32_t ResourceGroup;
    uint32_t InstanceHi;
    uint32_t InstanceLo;
    uint32_t ChunkOffset;
    uint32_t FileSize;
    uint32_t MemSize;
    uint32_t Compressed;
} Index;


