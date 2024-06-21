#include "filetypes/crcbin.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb");

    LoadCRCBinFile(f);

    fclose(f);

    return 0;
}

