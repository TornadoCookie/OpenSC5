#include <filetypes/package.h>
#include <cpl_raylib.h>
#include <stdlib.h>

static int compar_alphabetize(const void *p1, const void *p2)
{
    const char *s1 = *(const char **)p1;
    const char *s2 = *(const char **)p2;
    for (int i = 0; i < strlen(s1) && i < strlen(s2); i++)
    {
        if (s1[i] != s2[i])
        {
            return s1[i] - s2[i];
        }
    }

    return strlen(s1) - strlen(s2);
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("usage: opensc5 <SimCityData>\n");
        return 1;
    }

    printf("Treating %s as SimCityData...\n", argv[1]);

    FilePathList simcityData = LoadDirectoryFiles(argv[1]);

    printf("Found %d files.\n", simcityData.count);
    printf("Sorting packages...\n");

    qsort(simcityData.paths, simcityData.count, sizeof(char *), compar_alphabetize);

    printf("Loading Properties.txt...\n");

    PropertyNameList propNames = LoadPropertyNameList(TextFormat("%s/Config/Properties.txt", argv[1]));

    Package allGameData = { 0 };

    for (int i = 0; i < simcityData.count; i++)
    {
        printf("Loading %s...\n", simcityData.paths[i]);
    }

    return 0;
}
