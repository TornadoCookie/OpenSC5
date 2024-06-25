#ifndef _BNK_
#define _BNK_

#include <stdbool.h>

typedef struct BnkData {
    unsigned int pointsTo;
} BnkData;

BnkData LoadBnkData(unsigned char *data, int dataSize);

#endif
