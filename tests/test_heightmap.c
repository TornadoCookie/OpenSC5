#include "filetypes/heightmap.h"
#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

int main(int argc, char **argv)
{
    int dataSize;
    unsigned char *data = LoadFileData(argv[1], &dataSize);

    HeightmapData heightmapData = LoadHeightmapData(data, dataSize);

    ExportImage(heightmapData.heightmap, "heightmap.png");


    InitWindow(1280, 720, "heightmap Viewer");
    Mesh heightmapMesh = GenMeshHeightmap(heightmapData.heightmap, (Vector3){128, 64, 256});
    Material mat = LoadMaterialDefault();

    Texture2D tex = LoadTextureFromImage(heightmapData.heightmap);

    // Define the camera to look into our 3d world
	Camera camera = { 0 };
	camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    SetTargetFPS(60);

    DisableCursor();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        rlEnableWireMode();
        BeginMode3D(camera);
        DrawMesh(heightmapMesh, mat, MatrixTranslate(0, 0, 0));
        EndMode3D();
        rlDisableWireMode();
        UpdateCamera(&camera, CAMERA_FREE);

        DrawTexture(tex, 0, 0, WHITE);
        EndDrawing();
    }

    return 0;
}
