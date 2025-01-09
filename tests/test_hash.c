#include "hash.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        printf("\"%s\" -> %#lx\n", argv[i], TheHash(argv[i]));
    }

    return 0;
}

