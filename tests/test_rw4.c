#include "filetypes/rw4.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    int dataSize;
    unsigned char *data = LoadFileData(argv[1], &dataSize);

    LoadRW4Data(data, dataSize);

    return 0;
}
