#ifndef _CRCBIN_
#define _CRCBIN_

#include <stdio.h>
#include <stdint.h>

typedef struct CRCBinObject {
    uint32_t entryCount;
    uint32_t fileId;
    char **data1;
    uint32_t *data2;
    uint32_t *data3;
} CRCBinObject;

CRCBinObject LoadCRCBinFile(FILE *f);

#endif
