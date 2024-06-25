#ifndef _RW4_
#define _RW4_

#include <raylib.h>

typedef struct RW4Data {
    bool corrupted;
    Model model;
    ModelAnimation *animations;
    int animationCount;
} RW4Data;

RW4Data LoadRW4Data(unsigned char *data, int dataSize);

#endif
