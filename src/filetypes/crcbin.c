#include "filetypes/crcbin.h"
#include <stdint.h>
#include <stdlib.h>
#include <cpl_raylib.h>
#include <cpl_endian.h>
#include <string.h>
#include "crc32.h"

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

#define checkequals(thing, val) if (thing.always_##val != 0x##val) {printf("Info: "#thing".always_" #val " is instead %#x.\n", thing.always_##val);}

CRCBinObject LoadCRCBinFile(FILE *f)
{
    CRCBinHeader header = { 0 };
    CRCBinObject obj = { 0 };

    fread(&header, sizeof(CRCBinHeader), 1, f);

    header.always4_be = htobe32(header.always4_be);
    header.always16_be = htobe32(header.always16_be);
    header.entryCount = htobe32(header.entryCount);
    header.fileId = htobe32(header.fileId);

    printf("Always 4: %d\n", header.always4_be);
    checkequals(header, dac0f709);
    checkequals(header, 00800a00);
    printf("File Id: %d\n", header.fileId);
    checkequals(header, b03d420e);
    checkequals(header, 9c801200);
    printf("Entry Count: %d\n", header.entryCount);
    printf("Always 16: %d\n", header.always16_be);

    obj.entryCount = header.entryCount;
    obj.fileId = header.fileId;

    obj.data1 = malloc(sizeof(char*) * header.entryCount);
    obj.data2 = malloc(sizeof(uint32_t) * header.entryCount);
    obj.data3 = malloc(sizeof(uint32_t) * header.entryCount);

    for (int i = 0; i < header.entryCount; i++)
    {
        printf("\nEntry %d:\n", i);
        uint32_t length;
        fread(&length, sizeof(uint32_t), 1, f);
        if (feof(f))
        {
            printf("Unexpected end of file.\n");
            return (CRCBinObject){0};
        }
        length = htobe32(length);
        printf("Length: %d\n", length);
        char *str = malloc(length + 1);
        fread(str, 1, length, f);
        str[length] = 0;

        printf("%s\n", str);

        obj.data1[i] = str;
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
        obj.data2[i] = data;
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
        obj.data3[i] = data;
    }

    return obj;
}

bool CheckCRC(CRCBinObject obj, const char *file)
{
    printf("Checking CRC of %s against %d.\n", file, obj.fileId);
    printf("Searching for file in CRC object...\n");

    int fileIndex = -1;

    for (int i = 0; i < obj.entryCount; i++)
    {
        if (TextIsEqual(obj.data1[i], GetFileName(file)))
        {
            printf("Found: %d.\n", i);
            fileIndex = i;
        }
    }

    if (fileIndex == -1)
    {
        printf("Error: didn't find file in object.\n");
        return false;
    }

    int dataSize;
    unsigned char *data = LoadFileData(file, &dataSize);

    printf("Checking... seed=%#x\n", obj.data2[fileIndex]);
    unsigned int crc = calculate_crc32c(obj.data2[fileIndex], data, dataSize);

    if (crc == obj.data3[fileIndex])
    {
        printf("CRC OK %#X\n", crc);
    }
    else
    {
        printf("CRC Failed. Expected %#x, got %#x.\n", obj.data3[fileIndex], crc);
    }

}
