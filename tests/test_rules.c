#include "filetypes/rules.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    int dataSize;
    unsigned char *data = LoadFileData(argv[1], &dataSize);

    LoadRulesData(data, dataSize);

    return 0;
}
