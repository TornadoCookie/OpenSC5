#include "filetypes/rw4.h"
#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

int main(int argc, char **argv)
{
    int dataSize;
    unsigned char *data = LoadFileData(argv[1], &dataSize);

    InitWindow(1280, 720, "RW4 Test");

    RW4Data rw4data =  LoadRW4Data(data, dataSize);

    if (!rw4data.corrupted)
    {

        // Define the camera to look into our 3d world
	    Camera camera = { 0 };
	    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
	    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	    camera.fovy = 45.0f;                                // Camera field-of-view Y
	    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
        SetTargetFPS(60);

        Material mat = LoadMaterialDefault();
        mat.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

        //UploadMesh(&rw4data.model.meshes[0], false);

        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            rlEnableWireMode();

            BeginMode3D(camera);

            //DrawMesh(rw4data.model.meshes[0], mat, MatrixTranslate(0, 0, 0));

            EndMode3D();

            UpdateCamera(&camera, CAMERA_ORBITAL);

            EndDrawing();
        }
    }

    return 0;
}
