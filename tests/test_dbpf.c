#include "filetypes/package.h"

int main(int argc, char **argv)
{
    int dataSize;
    unsigned char *data = LoadFileData(argv[1], &dataSize);

    DecompressDBPF(data, dataSize, 1009);

    return 0;
}
