#ifndef _RAST_
#define _RAST_

#include <raylib.h>

typedef struct RastData {
    bool corrupted;
    Image img;
} RastData;

RastData LoadRastData(unsigned char *data, int dataSize);

#endif

