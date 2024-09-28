#include "filetypes/statefile.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    LoadStateFile(argv[1]);

    return 0;
}

