#include "filetypes/crcbin.h"
#include <stdint.h>
#include <stdlib.h>
#include <endian.h>

typedef struct CRCBinHeader {
    uint32_t always4_be;
    uint32_t always_dac0f709;
    uint32_t always_00800a00;
    uint32_t fileId;
    uint32_t always_b03d420e;
    uint32_t always_9c801200;
    uint32_t entryCount;
    uint32_t always16_be;
} CRCBinHeader;

typedef struct CRCBinFile {
    CRCBinHeader header;
    char **strEntries;
} CRCBinFile;

#define checkequals(thing, val) if (thing.always_##val != 0x##val) {printf("Info: "#thing".always_" #val " is instead %#x.\n", thing.always_##val);}

void LoadCRCBinFile(FILE *f)
{
    CRCBinFile file = { 0 };

    fread(&file.header, sizeof(CRCBinHeader), 1, f);

    file.header.always4_be = htobe32(file.header.always4_be);
    file.header.always16_be = htobe32(file.header.always16_be);
    file.header.entryCount = htobe32(file.header.entryCount);
    file.header.fileId = htobe32(file.header.fileId);

    printf("Always 4: %d\n", file.header.always4_be);
    checkequals(file.header, dac0f709);
    checkequals(file.header, 00800a00);
    printf("File Id: %d\n", file.header.fileId);
    checkequals(file.header, b03d420e);
    checkequals(file.header, 9c801200);
    printf("Entry Count: %d\n", file.header.entryCount);
    printf("Always 16: %d\n", file.header.always16_be);

    file.strEntries = malloc(sizeof(char*) * file.header.entryCount);

    for (int i = 0; i < file.header.entryCount; i++)
    {
        printf("\nEntry %d:\n", i);
        uint32_t length;
        fread(&length, sizeof(uint32_t), 1, f);
        if (feof(f))
        {
            printf("Unexpected end of file.\n");
            return;
        }
        length = htobe32(length);
        printf("Length: %d\n", length);
        char *str = malloc(length + 1);
        fread(str, 1, length, f);
        str[length] = 0;

        printf("%s\n", str);

        file.strEntries[i] = str;
    }

    struct {
        uint32_t always_b13d420e;
        uint32_t always_9c800a00;
        uint32_t entryCount2;
        uint32_t always4_be;
    } data2;

    fread(&data2, sizeof(data2), 1, f);

    printf("\nData 2:\n");

    checkequals(data2, b13d420e);
    checkequals(data2, 9c800a00);

    data2.entryCount2 = htobe32(data2.entryCount2);

    for (int i = 0; i < data2.entryCount2; i++)
    {
        uint32_t data;
        fread(&data, sizeof(uint32_t), 1, f);
        printf("Entry %d: %#x\n", i, data);
    }

    struct {
        uint32_t always_33ebc10e;
        uint32_t always_9c800a00;
        uint32_t entryCount3;
        uint32_t always4_be;
    } data3;

    fread(&data3, sizeof(data3), 1, f);

    printf("\nData 3:\n");

    checkequals(data3, 33ebc10e);
    checkequals(data3, 9c800a00);

    data3.entryCount3 = htobe32(data3.entryCount3);

    for (int i = 0; i < data3.entryCount3; i++)
    {
        uint32_t data;
        fread(&data, sizeof(uint32_t), 1, f);
        printf("Entry %d: %#x\n", i, data);
    }
}
