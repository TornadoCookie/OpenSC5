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

    SetTraceLogLevel(LOG_DEBUG);
    RW4Data rw4data =  LoadRW4Data(data, dataSize);
    SetTraceLogLevel(LOG_INFO);

    //if (!rw4data.corrupted)
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

        if (rw4data.type == RW4_TEXTURE)
        {
            rw4data.data.texData.tex = LoadTextureFromImage(rw4data.data.texData.img);
        }
        else if (rw4data.type == RW4_MODEL)
        {
            UploadMesh(&rw4data.data.mdlData.mdl.meshes[0], false);
        }

        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            if (rw4data.type == RW4_TEXTURE)
            {
                DrawTexture(rw4data.data.texData.tex, 0, 0, WHITE);
            }
            else if (rw4data.type == RW4_MODEL)
            {

                rlEnableWireMode();

                BeginMode3D(camera);

                DrawModel(rw4data.data.mdlData.mdl, (Vector3){0, 0, 0}, 1.0f, WHITE);

                //DrawMesh(rw4data.data.mdlData.mdl.meshes[0], mat, MatrixTranslate(0, 0, 0));

                EndMode3D();

                UpdateCamera(&camera, CAMERA_ORBITAL);
            }

            EndDrawing();
        }
    }

    return 0;
}
