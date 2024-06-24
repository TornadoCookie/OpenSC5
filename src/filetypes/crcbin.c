#include "filetypes/crcbin.h"
#include <stdint.h>
#include <stdlib.h>
#include <cpl_raylib.h>
#include <cpl_endian.h>
#include <string.h>

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

// 8-bit CRC using the polynomial x^8+x^6+x^3+x^2+1, 0x14D.
// Chosen based on Koopman, et al. (0xA6 in his notation = 0x14D >> 1):
// http://www.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf
//
// This implementation is reflected, processing the least-significant bit of the
// input first, has an initial CRC register value of 0xff, and exclusive-or's
// the final register value with 0xff. As a result the CRC of an empty string,
// and therefore the initial CRC value, is zero.
//
// The standard description of this CRC is:
// width=8 poly=0x4d init=0xff refin=true refout=true xorout=0xff check=0xd8
// name="CRC-8/KOOP"

static unsigned char const crc8_table[] = {
    0xea, 0xd4, 0x96, 0xa8, 0x12, 0x2c, 0x6e, 0x50, 0x7f, 0x41, 0x03, 0x3d,
    0x87, 0xb9, 0xfb, 0xc5, 0xa5, 0x9b, 0xd9, 0xe7, 0x5d, 0x63, 0x21, 0x1f,
    0x30, 0x0e, 0x4c, 0x72, 0xc8, 0xf6, 0xb4, 0x8a, 0x74, 0x4a, 0x08, 0x36,
    0x8c, 0xb2, 0xf0, 0xce, 0xe1, 0xdf, 0x9d, 0xa3, 0x19, 0x27, 0x65, 0x5b,
    0x3b, 0x05, 0x47, 0x79, 0xc3, 0xfd, 0xbf, 0x81, 0xae, 0x90, 0xd2, 0xec,
    0x56, 0x68, 0x2a, 0x14, 0xb3, 0x8d, 0xcf, 0xf1, 0x4b, 0x75, 0x37, 0x09,
    0x26, 0x18, 0x5a, 0x64, 0xde, 0xe0, 0xa2, 0x9c, 0xfc, 0xc2, 0x80, 0xbe,
    0x04, 0x3a, 0x78, 0x46, 0x69, 0x57, 0x15, 0x2b, 0x91, 0xaf, 0xed, 0xd3,
    0x2d, 0x13, 0x51, 0x6f, 0xd5, 0xeb, 0xa9, 0x97, 0xb8, 0x86, 0xc4, 0xfa,
    0x40, 0x7e, 0x3c, 0x02, 0x62, 0x5c, 0x1e, 0x20, 0x9a, 0xa4, 0xe6, 0xd8,
    0xf7, 0xc9, 0x8b, 0xb5, 0x0f, 0x31, 0x73, 0x4d, 0x58, 0x66, 0x24, 0x1a,
    0xa0, 0x9e, 0xdc, 0xe2, 0xcd, 0xf3, 0xb1, 0x8f, 0x35, 0x0b, 0x49, 0x77,
    0x17, 0x29, 0x6b, 0x55, 0xef, 0xd1, 0x93, 0xad, 0x82, 0xbc, 0xfe, 0xc0,
    0x7a, 0x44, 0x06, 0x38, 0xc6, 0xf8, 0xba, 0x84, 0x3e, 0x00, 0x42, 0x7c,
    0x53, 0x6d, 0x2f, 0x11, 0xab, 0x95, 0xd7, 0xe9, 0x89, 0xb7, 0xf5, 0xcb,
    0x71, 0x4f, 0x0d, 0x33, 0x1c, 0x22, 0x60, 0x5e, 0xe4, 0xda, 0x98, 0xa6,
    0x01, 0x3f, 0x7d, 0x43, 0xf9, 0xc7, 0x85, 0xbb, 0x94, 0xaa, 0xe8, 0xd6,
    0x6c, 0x52, 0x10, 0x2e, 0x4e, 0x70, 0x32, 0x0c, 0xb6, 0x88, 0xca, 0xf4,
    0xdb, 0xe5, 0xa7, 0x99, 0x23, 0x1d, 0x5f, 0x61, 0x9f, 0xa1, 0xe3, 0xdd,
    0x67, 0x59, 0x1b, 0x25, 0x0a, 0x34, 0x76, 0x48, 0xf2, 0xcc, 0x8e, 0xb0,
    0xd0, 0xee, 0xac, 0x92, 0x28, 0x16, 0x54, 0x6a, 0x45, 0x7b, 0x39, 0x07,
    0xbd, 0x83, 0xc1, 0xff};

#include <stddef.h>

// Return the CRC-8 of data[0..len-1] applied to the seed crc. This permits the
// calculation of a CRC a chunk at a time, using the previously returned value
// for the next seed. If data is NULL, then return the initial seed. See the
// test code for an example of the proper usage.
unsigned crc8(unsigned crc, unsigned char const *data, size_t len)
{
    if (data == NULL)
        return 0;
    crc &= 0xff;
    unsigned char const *end = data + len;
    while (data < end)
        crc = crc8_table[crc ^ *data++];
    return crc;
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
            printf("Found: %d.\n");
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
    unsigned int crc = crc8(obj.data2[fileIndex], data, dataSize);

    if (crc == obj.data3[fileIndex])
    {
        printf("CRC OK %#X\n", crc);
    }
    else
    {
        printf("CRC Failed. Expected %#x, got %#x.\n", obj.data3[fileIndex], crc);
    }

}
