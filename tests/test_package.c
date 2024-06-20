#include "filetypes/package.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    int dataSize;
    LoadPackageFile(LoadFileData(argv[1], &dataSize));

    return 0;
}

