#include <raylib.h>
#include "filetypes/package.h"
#include <raymath.h>

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
        GuiDrawRectangle(buttonBounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), GetColor(GuiGetStyle(TEXTBOX, BORDER + GuiGetState()*3)), selected?GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)):RAYWHITE);
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

static const char *PropValToString(PropVariable var, int i)
{
    switch (var.type)
    {
        case PROPVAR_BOOL: return var.values[i].b?"true":"false";
        case PROPVAR_INT32: return TextFormat("%d", var.values[i].int32);
        case PROPVAR_UINT32: return TextFormat("%u", var.values[i].uint32);
        case PROPVAR_FLOAT: return TextFormat("%f", var.values[i].f);
        case PROPVAR_STR8: return TextFormat("\"%s\"", var.values[i].string8);
        case PROPVAR_STRING: return TextFormat("\"%s\"", var.values[i].string);
        case PROPVAR_KEYS: return TextFormat("File: %#X, Type: %#X, Group:%#X", var.values[i].keys.file, var.values[i].keys.type, var.values[i].keys.group);
        case PROPVAR_TEXTS: return TextFormat("File spec: %#X, Identifier: %#X", var.values[i].texts.fileSpec, var.values[i].texts.identifier);
        case PROPVAR_VECT2: return TextFormat("{%f, %f}", var.values[i].vector2.x, var.values[i].vector2.y);
        case PROPVAR_VECT3: return TextFormat("{%f, %f, %f}", var.values[i].vector3.x, var.values[i].vector3.y, var.values[i].vector3.z);
        case PROPVAR_COLRGB: return TextFormat("%d, %d, %d", var.values[i].colorRGB.r*255, var.values[i].colorRGB.g*255, var.values[i].colorRGB.b*255);
        case PROPVAR_BBOX: return TextFormat("min {%f, %f, %f}, max {%f, %f %f}", var.values[i].bbox.min.x, var.values[i].bbox.min.y, var.values[i].bbox.min.z, 
                                             var.values[i].bbox.max.x, var.values[i].bbox.max.y, var.values[i].bbox.max.z);
        default: return "Unable to read type";
    }
}

static const char *PropVarTypeToString(unsigned int type)
{
    switch (type)
    {
        case PROPVAR_BOOL: return "BOOL";
        case PROPVAR_INT32: return "INT32";
        case PROPVAR_UINT32: return "UINT32";
        case PROPVAR_FLOAT: return "FLOAT";
        case PROPVAR_STR8: return "STRING8";
        case PROPVAR_STRING: return "STRING";
        case PROPVAR_KEYS: return "KEYS";
        case PROPVAR_TEXTS: return "TEXTS";
        case PROPVAR_VECT2: return "VECTOR2";
        case PROPVAR_VECT3: return "VECTOR3";
        case PROPVAR_COLRGB: return "COLORRGB";
        case PROPVAR_BBOX: return "BBOX";
        default: return "UNKNOWN";
    }
}

static int selectedPropVal;
static Rectangle propView;
static Vector2 propScroll;

static void DrawPackageEntry(PackageEntry entry)
{
    switch (entry.type)
    {
        case PKGENTRY_PROP:
        {
            PropData propData = entry.data.propData;
            GuiPanel((Rectangle){GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight()/2}, "Properties");

            GuiScrollPanel((Rectangle){GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight()/2}, "Properties", (Rectangle){GetScreenWidth()/2,RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*2,GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(propData.variableCount+1)}, &propScroll, &propView);

            BeginScissorMode(propView.x, propView.y, propView.width, propView.height);

            for (int i = 0; i < propData.variableCount; i++)
            {
                PropVariable var = propData.variables[i];
                ListRow row = { 0 };

                row.elementCount = 3;
                row.elementWidth = (float[3]){0.333, 0.333, 0.333};
                row.elementText = (const char *[3]){TextFormat("%#X", var.identifier), TextFormat("%#X (%s)", var.type, PropVarTypeToString(var.type)), TextFormat("%#X", var.count)};

                bool shouldToggleSelect = DrawListRow((Rectangle){
                    GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(i+2)+propScroll.y,
                    GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT
                }, row, i == selectedPropVal, true);

                if (shouldToggleSelect)
                {
                    if (i != selectedPropVal) selectedPropVal = i;
                    else selectedPropVal = -1;
                }
            }

            EndScissorMode();

            DrawListRow((Rectangle) {
                GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
            }, (ListRow) {
                3, (float[3]){0.333, 0.333, 0.333},
                (const char *[3]){"IDENTIFIER", "TYPE", "COUNT"}
            }, false, false);

            GuiPanel((Rectangle){GetScreenWidth()/2,GetScreenHeight()/2,GetScreenWidth()/2,GetScreenHeight()/2}, "Values");

            if (selectedPropVal != -1)
            {
                PropVariable var = propData.variables[selectedPropVal];
                
                for (int i = 0; i < var.count; i++)
                {
                    ListRow row = { 0 };
                    
                    row.elementCount = 1;
                    row.elementWidth = (float[1]){1.0};
                    row.elementText = (const char *[1]){PropValToString(var, i)};
                    
                    DrawListRow((Rectangle) {
                        GetScreenWidth()/2, GetScreenHeight()/2 + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(i+1),
                        GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                    }, row, false, false);
                }
            }
        } break;
        case PKGENTRY_JSON:
        case PKGENTRY_SCPT:
        {
            GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);   // WARNING: Word-wrap does not work as expected in case of no-top alignment
            GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);            // WARNING: If wrap mode enabled, text editing is not supported
            GuiTextBox((Rectangle){ GetScreenWidth()/2,0,GetScreenWidth()/2,GetScreenHeight() }, entry.data.scriptSource, strlen(entry.data.scriptSource), false);
            GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);
            GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
        } break;
        default:
        {
            DrawText("Unable to parse this data yet", GetScreenWidth()*3/4 - MeasureText("Unable to parse this data yet", 20)/2, GetScreenHeight()/2 - 10, 20, GRAY);
        } break;
    }
}

int main()
{
    Package loadedPkg = { 0 };
    bool hasLoadedPkg = false;

    int selectedPkgEntry = -1;
    Rectangle pkgEntryListView = {0};
    Vector2 pkgEntryListScroll = {0};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "OpenSC5 Editor");

    SetTargetFPS(60);

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

                    BeginDrawing();
                    ClearBackground(RAYWHITE);
                    DrawText("Loading...", GetScreenWidth() / 2 - MeasureText("Loading...", 20)/2, GetScreenHeight() / 2 - 10, 20, GRAY);
                    EndDrawing();
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
            GuiScrollPanel((Rectangle){0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT, GetScreenWidth()/2, GetScreenHeight()}, "Entries", (Rectangle){0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT, GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(loadedPkg.entryCount+1)}, &pkgEntryListScroll, &pkgEntryListView);

            BeginScissorMode(pkgEntryListView.x, pkgEntryListView.y, pkgEntryListView.width, pkgEntryListView.height);

            for (int i = 0; i < loadedPkg.entryCount; i++)
            {
                ListRow row = { 0 };
                PackageEntry entry = loadedPkg.entries[i];

                row.elementCount = 3;
                row.elementWidth = (float[3]){0.333, 0.333, 0.333};
                row.elementText = (const char*[3]){TextFormat("%#X (%s)", entry.type, PackageEntryTypeToString(entry.type)), TextFormat("%#X", entry.instance), TextFormat("%#X", entry.group)};

                bool shouldToggleSelect = DrawListRow((Rectangle) {
                    0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT * (i+2) + pkgEntryListScroll.y,
                    GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                }, row, i == selectedPkgEntry, true);

                if (shouldToggleSelect)
                {
                    selectedPropVal = -1;
                    propView = (Rectangle){0};
                    if (i != selectedPkgEntry) selectedPkgEntry = i;
                    else selectedPkgEntry = -1;
                }

            }

            EndScissorMode();

            DrawListRow((Rectangle) {
                0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
            }, (ListRow) {
                3, (float[3]){0.333, 0.333, 0.333},
                (const char *[3]){"TYPE", "INSTANCE", "GROUP"}
            }, false, false);

            if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))
            {
                selectedPkgEntry++;
                selectedPropVal = -1;
                propView = (Rectangle){0};
            }

            if ((IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) && selectedPkgEntry != 0)
            {
                selectedPkgEntry--;
                selectedPropVal = -1;
                propView = (Rectangle){0};
            }

            if (selectedPkgEntry != -1)
            {
                selectedPkgEntry = Clamp(selectedPkgEntry, 0, loadedPkg.entryCount-1);

                PackageEntry entry = loadedPkg.entries[selectedPkgEntry];

                if (entry.corrupted)
                {
                    DrawText("Corrupted.", GetScreenWidth()*3/4 - MeasureText("Corrupted.", 20)/2, GetScreenHeight()/2 - 10, 20, GRAY);
                }
                else
                {
                    DrawPackageEntry(entry);
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
