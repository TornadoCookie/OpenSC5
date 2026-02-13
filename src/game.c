#include <filetypes/package.h>
#include <cpl_raylib.h>
#include <stdlib.h>
#include <cpl_pthread.h>
#include <threadpool.h>
#include <rlWebKit.h>

#include "DBPFFileSystem.hpp"

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

static void loadPackage(const char *pkgFile, Package *allGameData)
{
    printf("Loading %s...\n", pkgFile);
    FILE *f = fopen(pkgFile, "r");
    if (!f)
    {
        perror(pkgFile);
        exit(EXIT_FAILURE);
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
        DrawText(TextFormat("%s: %d left.\n", pkgFile, GetThreadpoolTasksLeft()), 0, 0, 20, BLACK);
        EndDrawing();
    }
    pthread_join(thread, NULL);
    printf("Merging package into game...\n");
    MergePackages(allGameData, pkg);
    fclose(f);
}

#define UPDATER_WINDOW_W 900
#define UPDATER_WINDOW_H 576

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("usage: opensc5 <SimCityData>\n");
        return 1;
    }

    initWebkit(); // otherwise this breaks allocation

    printf("Treating %s as SimCityData...\n", argv[1]);

    SetTraceLogLevel(LOG_ERROR);

    FilePathList simcityData = LoadDirectoryFiles(argv[1]);

    printf("Found %d files.\n", simcityData.count);
    printf("Sorting packages...\n");

    qsort(simcityData.paths, simcityData.count, sizeof(char *), compar_alphabetize);

    printf("Loading Properties.txt...\n");

    PropertyNameList propNames = LoadPropertyNameList(TextFormat("%s/Config/Properties.txt", argv[1]));

    Package allGameData = { 0 };

    const char *appPkgFn = strdup(TextFormat("%s/SimCity_App.package", argv[1]));
    const char *lcPkgFn = strdup(TextFormat("%s/Locale/en-us/Data.package", argv[1]));
    const char *gamePkgFn = strdup(TextFormat("%s/SimCity_Game.package", argv[1]));

    SetTraceLogLevel(LOG_INFO);
    InitWindow(UPDATER_WINDOW_W, UPDATER_WINDOW_H, "OpenSC5 Launcher");
    SetWriteCorruptedPackageEntries(false);
    SetTryParseFilesInPackage(false);

    Package SimCity_App;

    loadPackage(appPkgFn, &allGameData);
    loadPackage(lcPkgFn, &allGameData);
    loadPackage(gamePkgFn, &allGameData);

    SetWebKitPackage(&allGameData);

    printf("Loaded %d package entries.\n", allGameData.entryCount);

    void *v = createView(UPDATER_WINDOW_W, UPDATER_WINDOW_H);
    setViewUrl(v, "game:///dbpf/Updater.html");

    // entrypoints:
    // Updater.html
    // Editor.html
    // Launcher3D_planar.html
    // WebKitPanel.html
    //

    Image img = GenImageColor(UPDATER_WINDOW_W, UPDATER_WINDOW_H, BLANK);
    Texture2D screenTex = LoadTextureFromImage(img);
    UnloadImage(img);

    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F5))
         {
             reload(v);
         }

        mousemove(v, GetMouseDelta().x, GetMouseDelta().y);

         int mouseButtonPressed = -1;
         bool mouseButtonState;

        // kMouseLeft = 0
        // kMouseMiddle = 1
        // kMouseRight = 2

         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
         {
             mouseButtonPressed = 0;
             mouseButtonState = true;
         }
         else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
         {
             mouseButtonPressed = 0;
             mouseButtonState = false;
         }

         if (mouseButtonPressed != -1)
         {
             mousebutton(v, GetMouseX(), GetMouseY(), mouseButtonPressed, mouseButtonState);
         }

         mousewheel(v, GetMouseX(), GetMouseY(), 0, GetMouseWheelMove()*60);

         int key = GetCharPressed();

         while(key > 0)
         {
             keyboard(v, key, true, true);

             key = GetCharPressed();
         }

         if (IsKeyPressed(KEY_ENTER))
         {
             keyboard(v, '\r', false, true);
         }


      BeginDrawing();
      ClearBackground(RAYWHITE);

      //BeginMode3D(camera);

      //DrawCubeWires((Vector3){0, 0, 0}, 1, 1, 1, RED);

      //EndMode3D();

      //update the things
      updateWebkit(v);
      updateView(v);

      updateGLTexture(v, screenTex);
      DrawTexture(screenTex, 0, 0, WHITE);
     
      //draw the things
      //drawCube();
      //drawInterface(v);      

      //Update screen
      EndDrawing();
    }

    destroyView(v);
    shutdownWebKit();

    return 0;
}
