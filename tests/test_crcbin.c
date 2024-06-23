#include "filetypes/crcbin.h"
#include <stdio.h>
#include <raylib.h>
#include <string.h>

int main(int argc, char **argv)
{
    FilePathList files = LoadDirectoryFiles(argv[1]);

    for (int i = 0; i < files.count; i++)
    {
        if (!IsFileExtension(files.paths[i], ".bin")) continue;
        if (strstr(files.paths[i], "Validate")) continue;
        printf("\nFile %s:\n", files.paths[i]);

        FILE *f = fopen(files.paths[i], "rb");

        CRCBinObject obj = LoadCRCBinFile(f);

        fclose(f);   
    }

    return 0;
}

