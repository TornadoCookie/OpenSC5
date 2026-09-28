#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <windows.h>

int main(int argc, char **argv)
{
    FILE *log = fopen("C:\\torigin.log", "w");

    fprintf(log, "torigin started @ %ld.\n", time(NULL));
    for (int i = 0; i < argc; i++)
    {
        fprintf(log, "argv[%d]: %s\n", i, argv[i]);
    }

    fprintf(log, "assuming SMOID 96\n");

    const HANDLE smoid = (HANDLE)96;
    const DWORD maxSize = 0x1000;

    void * mem = MapViewOfFile(smoid, FILE_MAP_READ, 0, 0, maxSize);

    if (mem == NULL)
    {
        fprintf(log, "Failed to get shared memory: %ld\n", GetLastError());
    }
    else
    {
        fprintf(log, "Got memory at address %p, dumping...\n", mem);
        FILE *memdump = fopen("C:\\to_memdump.bin", "wb");
        fwrite(mem, maxSize, 1, memdump);
        fclose(memdump);
    }

    return 0;
}
