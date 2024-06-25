#ifndef _BNK_
#define _BNK_

#include <stdbool.h>
#include <raylib.h>

typedef struct BnkData {
    unsigned int pointsTo;
    bool corrupted;

    int waveCount;
    Wave *waves;
} BnkData;

BnkData LoadBnkData(unsigned char *data, int dataSize);

#endif
