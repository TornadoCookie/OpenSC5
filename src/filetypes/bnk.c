#include "filetypes/bnk.h"
#include <stdint.h>

typedef struct BnkHeader {
    char signature[4]; // BKHD
    uint32_t size;     // 0x18000000
    uint32_t version;  // 0x41000000
    uint32_t pointsTo; // Identifier for a file in the same group.
    uint32_t unknown2; // 0x00000000
    uint32_t unknown3; // 0x00000000
    uint32_t unknown4; // 0x00000000
    uint32_t unknown5; // 0x00000000
} BnkHeader;

BnkData LoadBnkData(unsigned char *data, int dataSize)
{
    BnkData bnkData = { 0 };
    BnkHeader *header = (BnkHeader*)data;

    bnkData.pointsTo = header->pointsTo;

    return bnkData;
}
