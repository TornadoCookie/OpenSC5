#include <raylib.h>
#include "filetypes/package.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

typedef struct ListRow {
    int elementCount;
    float *elementWidth;
    const char **elementText;
} ListRow;

static bool DrawListRow(Rectangle bounds, ListRow row, bool selected, bool canSelect)
{
    int xOff = 0;
    bool focused = false;
    bool pressed = false;

    if (canSelect && CheckCollisionPointRec(GetMousePosition(), bounds)) 
    {
        focused = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            pressed = true;
        }
    }

    if (selected)
    {
        GuiSetState(STATE_PRESSED);
    }
    else if (focused)
    {
        GuiSetState(STATE_FOCUSED);
    }

    for (int i = 0; i < row.elementCount; i++)
    {
        Rectangle buttonBounds = (Rectangle){
            bounds.x + xOff,
            bounds.y,
            bounds.width * row.elementWidth[i],
            bounds.height
        };
        GuiDrawRectangle(buttonBounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), GetColor(GuiGetStyle(TEXTBOX, BORDER + GuiGetState()*3)), selected?GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)):BLANK);
        GuiLabel(buttonBounds, row.elementText[i]);
        xOff += bounds.width * row.elementWidth[i];
    }

    if (selected || focused || pressed)
    {
        GuiSetState(STATE_NORMAL);
    }
    return pressed;
}

static const char *PackageEntryTypeToString(unsigned int type)
{
    switch (type)
    {
        case PKGENTRY_PROP: return "PROP";
        case PKGENTRY_SCPT: return "SCPT";
        case PKGENTRY_RULE: return "RULE";
        case PKGENTRY_JSON: return "JSON";
        case PKGENTRY_RAST: return "RAST";
        default: return "UNKN";
    }
}

int main()
{
    Package loadedPkg = { 0 };
    bool hasLoadedPkg = false;

    int selectedPkgEntry = -1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "OpenSC5 Editor");

    while (!WindowShouldClose())
    {
        // Update
        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count == 1)
            {
                const char *path = droppedFiles.paths[0];

                if (IsFileExtension(path, ".package"))
                {
                    if (hasLoadedPkg)
                    {
                        UnloadPackageFile(loadedPkg);
                    }
                    hasLoadedPkg = true;
                    FILE *f = fopen(path, "rb");
                    loadedPkg = LoadPackageFile(f);
                    fclose(f);
                }
            }

            UnloadDroppedFiles(droppedFiles);
        }

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (hasLoadedPkg)
        {
            GuiPanel((Rectangle){0, 0, GetScreenWidth()/2, GetScreenHeight()}, "Entries");
            DrawListRow((Rectangle) {
                0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
            }, (ListRow) {
                3, (float[3]){0.333, 0.333, 0.333},
                (const char *[3]){"TYPE", "INSTANCE", "GROUP"}
            }, false, false);

            for (int i = 0; i < loadedPkg.entryCount; i++)
            {
                ListRow row = { 0 };
                PackageEntry entry = loadedPkg.entries[i];

                row.elementCount = 3;
                row.elementWidth = (float[3]){0.333, 0.333, 0.333};
                row.elementText = (const char*[3]){TextFormat("%#X (%s)", entry.type, PackageEntryTypeToString(entry.type)), TextFormat("%#X", entry.instance), TextFormat("%#X", entry.group)};

                bool shouldToggleSelect = DrawListRow((Rectangle) {
                    0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT * (i+2),
                    GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                }, row, i == selectedPkgEntry, true);

                if (shouldToggleSelect)
                {
                    if (i != selectedPkgEntry) selectedPkgEntry = i;
                    else selectedPkgEntry = -1;
                }

            }
        }
        else
        {
            DrawText("Drop a .PACKAGE file to start", GetScreenWidth() / 2 - MeasureText("Drop a .PACKAGE file to start", 20)/2, GetScreenHeight() / 2 - 10, 20, GRAY);
        }

        EndDrawing();
    }

    return 0;
}
