#include "filetypes/rast.h"
#include <stdio.h>
#include <raylib.h>

int main(int argc, char **argv)
{
    int dataSize;
    unsigned char *data = LoadFileData(argv[1], &dataSize);

    RastData rastData = LoadRastData(data, dataSize);


    InitWindow(1280, 720, "Rast Viewer");
    Texture2D tex = LoadTextureFromImage(rastData.img);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(tex, 0, 0, WHITE);
        EndDrawing();
    }

    return 0;
}
