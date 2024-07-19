#include "filetypes/sdelta.h"
#include <stdio.h>
#include <cpl_endian.h>
#include <stdint.h>

typedef struct SDeltaEntry {
    uint32_t unknown1; // some kind of class name
    uint32_t unknown2; // always null
    uint32_t unknown3; // always 1
    uint32_t unknown4; // some kind of instance id
    uint32_t unknown5; // some kind of flags
    uint32_t unknown6; // always null
} SDeltaEntry;

typedef struct SDeltaHeader {
    uint32_t unknown1;
    uint32_t unknown2; // always FF010301
    uint32_t unknown3; // entry count?
    uint32_t unknown4; // always 1
} SDeltaHeader;

SDelta LoadSDeltaFile(const char *filename)
{
    SDelta sdelta = { 0 };

    FILE *f = fopen(filename, "rb");

    SDeltaHeader header = { 0 };
    fread(&header, sizeof(SDeltaHeader), 1, f);

    header.unknown1 = htobe32(header.unknown1);
    header.unknown2 = htobe32(header.unknown2);
    header.unknown3 = htobe32(header.unknown3);
    header.unknown4 = htobe32(header.unknown4);

    printf("Unknown 1: %#x\n", header.unknown1);

    if (header.unknown2 != 0xFF010301)
    {
        printf("Unknown 2 is not 0xFF010301, is %#x\n", header.unknown2);
    }

    printf("Unknown 3: %#x\n", header.unknown3);
    
    if (header.unknown4 != 0x00000001)
    {
        printf("Unknown 4 is not 0x00000001, is %#x\n", header.unknown4);
    }

    int entryN = 0;

    while (!feof(f))
    {
        entryN++;
        printf("\nEntry %d:\n", entryN);
        SDeltaEntry entry;

        fread(&entry, sizeof(SDeltaEntry), 1, f);

        entry.unknown1 = htobe32(entry.unknown1);
        entry.unknown3 = htobe32(entry.unknown3);
        entry.unknown5 = htobe32(entry.unknown5);
        entry.unknown6 = htobe32(entry.unknown6);

        printf("Unknown 1: %#x\n", entry.unknown1);

        if (entry.unknown2 != 0x00000000)
        {
            printf("Unknown 2 not 0x00000000, is %#x\n", entry.unknown2);
        }

        if (entry.unknown3 != 0x00000001)
        {
            printf("Unknown 3 not 0x00000001, is %#x\n", entry.unknown3);
        }
        
        printf("Unknown 4: %#x\n", entry.unknown4);
        printf("Unknown 5: %#x\n", entry.unknown5);

        if (entry.unknown6 != 0x00000000)
        {
            printf("Unknown 6 not 0x00000000, is %#x\n", entry.unknown6);
        }

        if (entry.unknown3 == 0x69696969)
        {
            printf("LOL Nice.\n");
            uint32_t unknown[2];

            fread(unknown, sizeof(uint32_t), 2, f);
        }
    }

    fclose(f);

    return sdelta;
}
