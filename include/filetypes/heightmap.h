#ifndef _HEIGHTMAP_
#define _HEIGHTMAP_

#include <cpl_raylib.h>

typedef struct HeightmapData {
    bool corrupted;
    Image heightmap;
} HeightmapData;

HeightmapData LoadHeightmapData(unsigned char *data, int dataSize);

#endif
