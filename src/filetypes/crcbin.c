#include "filetypes/crcbin.h"
#include <stdint.h>
#include <stdlib.h>
#include <endian.h>

typedef struct CRCBinHeader {
    uint32_t always4_be;
    uint32_t always_09fcc0da;
    uint32_t always_000a8000;
    uint32_t unk1; // 1335f58d if it is the first item in the CRC
    uint32_t always_0e423db0;
    uint32_t _0012809c;
    uint32_t entryCount;
    uint32_t always16_be;
} CRCBinHeader;

typedef struct CRCBinFile {
    CRCBinHeader header;
    char **strEntries;
} CRCBinFile;

void LoadCRCBinFile(FILE *f)
{
    CRCBinFile file = { 0 };

    fread(&file.header, sizeof(CRCBinHeader), 1, f);

    file.header.always4_be = htobe32(file.header.always4_be);
    file.header.always16_be = htobe32(file.header.always16_be);
    file.header.entryCount = htobe32(file.header.entryCount);

    printf("Always 4: %d\n", file.header.always4_be);
    printf("Always 09fcc0da: %#x\n", file.header.always_09fcc0da);
    printf("Always 000a8000: %#x\n", file.header.always_000a8000);
    printf("Unknown 1: %#x\n", file.header.unk1);
    printf("Always 0e423db0: %#x\n", file.header.always_0e423db0);
    printf("Always 0012809c: %#x\n", file.header._0012809c);
    printf("Entry Count: %d\n", file.header.entryCount);
    printf("Always 16: %d\n", file.header.always16_be);

    file.strEntries = malloc(sizeof(char*) * file.header.entryCount);

    for (int i = 0; i < file.header.entryCount; i++)
    {
        printf("\nEntry %d:\n", i);
        uint32_t length;
        fread(&length, sizeof(uint32_t), 1, f);
        length = htobe32(length);
        printf("Length: %d\n", length);
        char *str = malloc(length + 1);
        fread(str, 1, length, f);
        str[length] = 0;

        printf("%s\n", str);

        file.strEntries[i] = str;
    }
}
