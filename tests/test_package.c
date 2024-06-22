#include "filetypes/package.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb");

    if (!f)
    {
        perror("File error");
        return 1;
    }

    LoadPackageFile(f);

    fclose(f);

    return 0;
}

