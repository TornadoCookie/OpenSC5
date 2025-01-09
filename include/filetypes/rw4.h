#ifndef _RW4_
#define _RW4_

#include <cpl_raylib.h>

typedef enum {
    RW4_INVALID,
    RW4_TEXTURE,
} RW4DataType;

typedef struct RW4Data {
    bool corrupted;
    RW4DataType type;

    union {
        struct {
            Image img;
            Texture2D tex;
        } texData;
    } data;

    //Model model;
    //ModelAnimation *animations;
    //int animationCount;
} RW4Data;

RW4Data LoadRW4Data(unsigned char *data, int dataSize);

#endif
