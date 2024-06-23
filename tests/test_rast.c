#include "filetypes/rast.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    int dataSize;
    unsigned char *data = LoadFileData(argv[1], &dataSize);

    LoadRastData(data, dataSize);

    return 0;
}
