#include <filetypes/package.h>
#include <cpl_raylib.h>
#include <stdlib.h>
#include <cpl_pthread.h>
#include <threadpool.h>

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

typedef struct LoadPackageFileAsyncArgs {
    FILE *f;
    Package *pkg;
    bool done;
} LoadPackageFileAsyncArgs;

static void *loadPackageFile_async(void *param)
{
    LoadPackageFileAsyncArgs *args = param;
    args->done = false;
    *args->pkg = LoadPackageFile(args->f);
    args->done = true;
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

    SetTraceLogLevel(LOG_INFO);
    InitWindow(1280, 720, "OpenSC5 Launcher");
    SetWriteCorruptedPackageEntries(false);

    for (int i = 0; i < simcityData.count; i++)
    {
        if (!IsFileExtension(simcityData.paths[i], ".package")) continue;
        printf("Loading %s...\n", simcityData.paths[i]);
        FILE *f = fopen(simcityData.paths[i], "r");
        if (!f)
        {
            perror(simcityData.paths[i]);
            continue;
        }
        pthread_t thread;
        LoadPackageFileAsyncArgs args;
        args.done = false;
        args.f = f;
        Package pkg;
        args.pkg = &pkg;
        pthread_create(&thread, NULL, loadPackageFile_async, &args);
        while (!args.done)
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText(TextFormat("%s: %d left.\n", simcityData.paths[i], GetThreadpoolTasksLeft()), 0, 0, 20, BLACK);
            EndDrawing();
        }
        pthread_join(thread, NULL);
        printf("Merging package into game...\n");
        MergePackages(&allGameData, pkg);
        fclose(f);
    }

    printf("Loaded %d package entries.\n", allGameData.entryCount);

    return 0;
}
