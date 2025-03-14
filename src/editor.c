#include <raylib.h>
#include "filetypes/package.h"
#include <raymath.h>
#include <threadpool.h>
#include <cpl_pthread.h>
#include <getopt.h>
#include "filetypes/prop.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

static Package loadedPkg = { 0 };
static int selectedPkgEntry = -1;

typedef struct {
    // Window management variables
    bool windowActive;
    Rectangle windowBounds;
    Vector2 panOffset;
    bool dragMode;
    bool supportDrag;

    // Search variables;
    bool searchInstance;
    bool searchGroup;
    bool searchType;

    unsigned int instance;
    unsigned int group;
    unsigned int type;

    int resultCount;
    int *results;

    int index;

    char instanceIdStr[256];
    char groupIdStr[256];
    char typeIdStr[256];

    bool instanceEditMode;
    bool groupEditMode;
    bool typeEditMode;

} GuiWindowFindDialogState;

GuiWindowFindDialogState InitGuiWindowFindDialog()
{
    GuiWindowFindDialogState state = { 0 };

    // Init window data
    state.windowBounds = (Rectangle){ GetScreenWidth()/2 - 440/2, GetScreenHeight()/2 - 310/2, 440, 310 };
    state.windowActive = false;
    state.supportDrag = true;
    state.dragMode = false;
    state.panOffset = (Vector2){ 0, 0 };

    state.searchInstance = false;
    state.searchGroup = false;
    state.searchType = false;

    state.results = NULL;
    state.resultCount = 0;

    state.index = 0;
    
    state.instanceEditMode = false;
    state.groupEditMode = false;
    state.typeEditMode = false;

    return state;
}

// Update and draw file dialog
void GuiWindowFindDialog(GuiWindowFindDialogState *state)
{
    if (state->windowActive)
    {
        // Update window dragging
        //----------------------------------------------------------------------------------------
        if (state->supportDrag)
        {
            Vector2 mousePosition = GetMousePosition();

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Window can be dragged from the top window bar
                if (CheckCollisionPointRec(mousePosition, (Rectangle){ state->windowBounds.x, state->windowBounds.y, (float)state->windowBounds.width, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT }))
                {
                    state->dragMode = true;
                    state->panOffset.x = mousePosition.x - state->windowBounds.x;
                    state->panOffset.y = mousePosition.y - state->windowBounds.y;
                }
            }

            if (state->dragMode)
            {
                state->windowBounds.x = (mousePosition.x - state->panOffset.x);
                state->windowBounds.y = (mousePosition.y - state->panOffset.y);

                // Check screen limits to avoid moving out of screen
                if (state->windowBounds.x < 0) state->windowBounds.x = 0;
                else if (state->windowBounds.x > (GetScreenWidth() - state->windowBounds.width)) state->windowBounds.x = GetScreenWidth() - state->windowBounds.width;

                if (state->windowBounds.y < 0) state->windowBounds.y = 0;
                else if (state->windowBounds.y > (GetScreenHeight() - state->windowBounds.height)) state->windowBounds.y = GetScreenHeight() - state->windowBounds.height;

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) state->dragMode = false;
            }
        }

        // Draw window and controls
        //----------------------------------------------------------------------------------------
        state->windowActive = !GuiWindowBox(state->windowBounds, "#42# Find");

        GuiCheckBox((Rectangle){state->windowBounds.x + 8, state->windowBounds.y + 32, 24, 24}, "Match Instance ID", &state->searchInstance);
        GuiCheckBox((Rectangle){state->windowBounds.x + 8, state->windowBounds.y + 64, 24, 24}, "Match Group ID", &state->searchGroup);
        GuiCheckBox((Rectangle){state->windowBounds.x + 8, state->windowBounds.y + 96, 24, 24}, "Match Type ID", &state->searchType);

        if (GuiTextBox((Rectangle){state->windowBounds.x + 168, state->windowBounds.y + 32, 240, 24}, state->instanceIdStr, 256, state->instanceEditMode)) state->instanceEditMode = !state->instanceEditMode;
        if (GuiTextBox((Rectangle){state->windowBounds.x + 168, state->windowBounds.y + 64, 240, 24}, state->groupIdStr, 256, state->groupEditMode)) state->groupEditMode = !state->groupEditMode;
        if (GuiTextBox((Rectangle){state->windowBounds.x + 168, state->windowBounds.y + 96, 240, 24}, state->typeIdStr, 256, state->typeEditMode)) state->typeEditMode = !state->typeEditMode;
        
        if (GuiButton((Rectangle){state->windowBounds.x + 8, state->windowBounds.y + 280, 120, 24}, "FIND NEXT"))
        {
            free(state->results);
            state->results = SearchPackage(loadedPkg, (PackageSearchParams) {
                state->searchInstance, state->searchGroup, state->searchType,
                state->instanceIdStr, state->groupIdStr, state->typeIdStr
            }, &state->resultCount);
            if (state->index < state->resultCount) state->index++;
        }

        if (GuiButton((Rectangle){state->windowBounds.x + 136, state->windowBounds.y + 280, 120, 24}, "FIND PREVIOUS"))
        {
            if (state->index > 0) state->index--;
        }

        GuiLabel((Rectangle){state->windowBounds.x + 8, state->windowBounds.y + 248, 120, 24}, TextFormat("%d Results", state->resultCount));

        GuiSpinner((Rectangle){state->windowBounds.x + 312, state->windowBounds.y + 280, 120, 24}, "", &state->index, 0, state->resultCount - 1, false);

        if (state->results) selectedPkgEntry = state->results[state->index];
    }
}

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

    if (bounds.y + bounds.height < 0 || bounds.y > GetScreenHeight()) return false;

    if (canSelect && CheckCollisionPointRec(GetMousePosition(), bounds) && GetMousePosition().y > RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*2 && !GuiIsLocked()) 
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
        case PKGENTRY_TEXT: return "TEXT";
        case PKGENTRY_PNG: return "PNG";
        case PKGENTRY_JSN8: return "JSON8";
        case PKGENTRY_BNK: return "BNK";
        case PKGENTRY_CSS: return "CSS";
        case PKGENTRY_RW4: return "RW4";
        case PKGENTRY_GIF: return "GIF";
        case PKGENTRY_MOV: return "MOV";
        case PKGENTRY_EXIF: return "EXIF";
        case PKGENTRY_SWB: return "SWB";
        case PKGENTRY_HTML: return "HTML";
        case PKGENTRY_ER2: return "ER2";
        case PKGENTRY_TTF: return "TTF";
        default: return "UNKN";
    }
}

static const char *PropValToString(PropVariable var, int i)
{
    switch (var.type)
    {
        case PROPVAR_BOOL: return var.values[i].b?"true":"false";
        case PROPVAR_INT32: return TextFormat("%d", var.values[i].int32);
        case PROPVAR_UINT32: return TextFormat("%#X", var.values[i].uint32);
        case PROPVAR_FLOAT: return TextFormat("%f", var.values[i].f);
        case PROPVAR_STR8: return TextFormat("\"%s\"", var.values[i].string8);
        case PROPVAR_STRING: return TextFormat("\"%s\"", var.values[i].string);
        case PROPVAR_KEYS: return TextFormat("File: %#X, Type: %#X, Group:%#X", var.values[i].keys.file, var.values[i].keys.type, var.values[i].keys.group);
        case PROPVAR_TEXTS: return TextFormat("File spec: %#X, Identifier: %#X", var.values[i].texts.fileSpec, var.values[i].texts.identifier);
        case PROPVAR_VECT2: return TextFormat("{%f, %f}", var.values[i].vector2.x, var.values[i].vector2.y);
        case PROPVAR_VECT3: return TextFormat("{%f, %f, %f}", var.values[i].vector3.x, var.values[i].vector3.y, var.values[i].vector3.z);
        case PROPVAR_COLRGB: return TextFormat("%d, %d, %d", var.values[i].colorRGB.r*255, var.values[i].colorRGB.g*255, var.values[i].colorRGB.b*255);
        case PROPVAR_CRGBA: return TextFormat("%d, %d, %d, %d", var.values[i].colorRGBA.r*255, var.values[i].colorRGBA.g*255, var.values[i].colorRGBA.b*255, var.values[i].colorRGBA.a*255);
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
        case PROPVAR_CRGBA: return "COLORRGBA";
        case PROPVAR_TRANS: return "TRANSFORM";
        case PROPVAR_BBOX: return "BBOX";
        default: return "UNKNOWN";
    }
}

static int selectedPropVal;
static Rectangle propView;
static Vector2 propScroll;
static Rectangle propValView;
static Vector2 propValScroll;

static Rectangle srcView;
static Vector2 srcScroll;

static int currentGifFrame;

static Rectangle bnkView;
static Vector2 bnkScroll;
static Sound bnkSound;

static void DrawPackageEntry(PackageEntry entry, PropertyNameList nameList)
{
    switch (entry.type)
    {
        case PKGENTRY_PROP:
        {
            PropData propData = entry.data.propData;
            GuiPanel((Rectangle){GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight()/2}, "Properties");

            GuiScrollPanel(
                (Rectangle){GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight()/2},
                "Properties",
                (Rectangle){GetScreenWidth()/2,RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*2,GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(propData.variableCount+1)},
                &propScroll,
                &propView);

            BeginScissorMode(propView.x, propView.y, propView.width, propView.height);

            for (int i = 0; i < propData.variableCount; i++)
            {
                PropVariable var = propData.variables[i];
                ListRow row = { 0 };

                row.elementCount = 3;
                row.elementWidth = (float[3]){0.333, 0.333, 0.333};
                row.elementText = (const char *[3]){TextFormat("%#X", var.identifier), TextFormat("%#X (%s)", var.type, PropVarTypeToString(var.type)), TextFormat("%#X", var.count)};

                for (int j = 0; j < nameList.propCount; j++)
                {
                    if (var.identifier == nameList.propIds[j])
                    {
                        row.elementText[0] = TextFormat("%#X (%s)", var.identifier, nameList.propNames[j]);
                    }
                }

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

            GuiScrollPanel(
                (Rectangle){GetScreenWidth()/2,GetScreenHeight()/2,GetScreenWidth()/2,GetScreenHeight()/2},
                "Values",
                (Rectangle){GetScreenWidth()/2,GetScreenHeight()/2+RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*2,GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(propData.variableCount+1)},
                &propValScroll,
                &propValView);

            BeginScissorMode(propValView.x, propValView.y, propValView.width, propValView.height);

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
                        GetScreenWidth()/2, GetScreenHeight()/2 + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(i+1)+propValScroll.y,
                        GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                    }, row, false, false);
                }
            }

            EndScissorMode();
        } break;
        case PKGENTRY_ER2:
        case PKGENTRY_HTML:
        case PKGENTRY_CSS:
        case PKGENTRY_TEXT:
        case PKGENTRY_JSON:
        case PKGENTRY_JSN8:
        case PKGENTRY_SCPT:
        {
            GuiScrollPanel(
                (Rectangle){GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight()},
                "Source",
                GetTextBounds(TEXTBOX, (Rectangle){ GetScreenWidth()/2,RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,GetScreenWidth()/2,GetScreenHeight() }), &srcScroll, &srcView);

            BeginScissorMode(srcView.x, srcView.y, srcView.width, srcView.height);

            GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);   // WARNING: Word-wrap does not work as expected in case of no-top alignment
            GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);            // WARNING: If wrap mode enabled, text editing is not supported
            GuiTextBox((Rectangle){ GetScreenWidth()/2,RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + srcScroll.y,GetScreenWidth()/2,GetScreenHeight() }, entry.data.scriptSource, strlen(entry.data.scriptSource), false);
            GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);
            GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);

            EndScissorMode();
        } break;
        case PKGENTRY_GIF:
        {
            currentGifFrame++;
            if (currentGifFrame > entry.data.gifData.frameCount) currentGifFrame = 0;
            unsigned int nextFrameDataoffset = entry.data.gifData.img.width*entry.data.gifData.img.height*4*currentGifFrame;
            UpdateTexture(entry.data.gifData.tex, ((unsigned char *)entry.data.gifData.img.data) + nextFrameDataoffset);
        }
        case PKGENTRY_RAST:
        case PKGENTRY_PNG:
        {
            DrawTexture(entry.data.imgData.tex, GetScreenWidth()/2, 0, WHITE);
        } break;
        case PKGENTRY_BNK:
        {
            GuiPanel((Rectangle){GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight()}, "Soundbank entries");

            GuiScrollPanel(
                (Rectangle){GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight()},
                "Soundbank entries",
                (Rectangle){GetScreenWidth()/2,RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*2,GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(entry.data.bnkData.waveCount+1)},
                &bnkScroll,
                &bnkView);

            BeginScissorMode(bnkView.x, bnkView.y, bnkView.width, bnkView.height);

            for (int i = 0; i < entry.data.bnkData.waveCount; i++)
            {
                ListRow row = { 0 };
                    
                row.elementCount = 1;
                row.elementWidth = (float[1]){1.0};
                row.elementText = (const char *[1]){TextFormat("Entry %d", i)};
                    
                bool clicked = DrawListRow((Rectangle) {
                    GetScreenWidth()/2, GetScreenHeight()/2 + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(i+1)+bnkScroll.y,
                    GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                }, row, false, true);

                if (clicked)
                {
                    if (IsSoundValid(bnkSound))
                    {
                        UnloadSound(bnkSound);
                    }

                    bnkSound = LoadSoundFromWave(entry.data.bnkData.waves[i]);
                    PlaySound(bnkSound);
                }
            }

            EndScissorMode();
        } break;
        case PKGENTRY_RW4:
        {
           if (entry.data.rw4Data.type == RW4_TEXTURE)
           {
               DrawTexture(entry.data.rw4Data.data.texData.tex, GetScreenWidth()/2, 0, WHITE);
           }
        } break;
        default:
        {
            DrawText("Unable to render this data yet", GetScreenWidth()*3/4 - MeasureText("Unable to render this data yet", 20)/2, GetScreenHeight()/2 - 10, 20, GRAY);
        } break;
    }
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

typedef enum {
    EXPORT_PACKAGE_ENTRY,
    EXPORT_PACKAGE,
    IMPORT_FILE_OVERWRITE,
    EXPORT_PACKAGE_ENTRY_COMPRESSED,
} FileDialogReason;

static FileDialogReason fileDialogReason;

int main(int argc, char **argv)
{
    bool hasLoadedPkg = false;

    Rectangle pkgEntryListView = {0};
    Vector2 pkgEntryListScroll = {0};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "OpenSC5 Editor");

    SetTargetFPS(60);

    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    GuiWindowFindDialogState findDialogState = InitGuiWindowFindDialog();

    if (argc > 1 && !strcmp(argv[1], "-debug")) SetTraceLogLevel(LOG_DEBUG); // bit of a hack but as we have only one command-line option it should be fine
                                                                             // too lazy to use getopt

    PropertyNameList nameList = LoadPropertyNameList("Properties.txt");
    const char **names = NULL; 

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
                    LoadPackageFileAsyncArgs args;
                    args.f = f;
                    args.pkg = &loadedPkg;
                    args.done = false;
                    pthread_t thread;
                    pthread_create(&thread, NULL, loadPackageFile_async, &args);

                    while (!args.done)
                    {
                        BeginDrawing();
                        ClearBackground(RAYWHITE);
                        DrawText(TextFormat("Loading... %d left", GetThreadpoolTasksLeft()), GetScreenWidth() / 2 - MeasureText("Loading...", 20)/2, GetScreenHeight() / 2 - 10, 20, GRAY);
                        EndDrawing();
                    }

                    fclose(f);

                    free(names);
                    names = calloc(loadedPkg.entryCount, sizeof(char *));

                    for (int i = 0; i < loadedPkg.entryCount; i++)
                    {
                        PackageEntry entry = loadedPkg.entries[i];
                        if (entry.type == PKGENTRY_RAST || entry.type == PKGENTRY_PNG || entry.type == PKGENTRY_GIF && !entry.corrupted)
                        {
                            loadedPkg.entries[i].data.imgData.tex = LoadTextureFromImage(entry.data.imgData.img);
                        }

                        if (entry.type == PKGENTRY_RW4 || !entry.corrupted && entry.data.rw4Data.type == RW4_TEXTURE)
                        {
                            loadedPkg.entries[i].data.rw4Data.data.texData.tex = LoadTextureFromImage(entry.data.rw4Data.data.texData.img);
                        }
                        
                        for (int j = 0; j < nameList.propCount; j++)
                        {
                            if (entry.instance == nameList.propIds[j])
                            {
                                names[i] = nameList.propNames[j];
                            }
                        }
                    }


                }
            }

            UnloadDroppedFiles(droppedFiles);
        }

        if (fileDialogState.SelectFilePressed)
        {
            switch (fileDialogReason)
            {
                case EXPORT_PACKAGE:
                {
                    ExportPackage(loadedPkg, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                    fileDialogState.SelectFilePressed = false;
                } break;
                case EXPORT_PACKAGE_ENTRY:
                {
                    ExportPackageEntry(loadedPkg.entries[selectedPkgEntry], TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                    fileDialogState.SelectFilePressed = false;
                } break;
                case IMPORT_FILE_OVERWRITE:
                {
                    const char *fname = TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText);
                    loadedPkg.entries[selectedPkgEntry].dataRaw = LoadFileData(fname, &loadedPkg.entries[selectedPkgEntry].dataRawSize);
                    fileDialogState.SelectFilePressed = false;
                } break;
                case EXPORT_PACKAGE_ENTRY_COMPRESSED:
                {
                    SaveFileData(TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText), loadedPkg.entries[selectedPkgEntry].dataCompressed, loadedPkg.entries[selectedPkgEntry].dataCompressedSize);
                    fileDialogState.SelectFilePressed = false;
                } break;
            }
        }

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (fileDialogState.windowActive) GuiLock();

        if (hasLoadedPkg)
        {
            if (GuiButton((Rectangle){32, 0, 100, 24}, "Export Package"))
            {
                fileDialogState.saveFileMode = true;
                fileDialogState.windowActive = true;
                fileDialogReason = EXPORT_PACKAGE;
            }

            GuiScrollPanel((Rectangle){0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT, GetScreenWidth()/2, GetScreenHeight()}, "Entries", (Rectangle){0, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT, GetScreenWidth()/2, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*(loadedPkg.entryCount+1)}, &pkgEntryListScroll, &pkgEntryListView);

            BeginScissorMode(pkgEntryListView.x, pkgEntryListView.y, pkgEntryListView.width, pkgEntryListView.height);

            for (int i = 0; i < loadedPkg.entryCount; i++)
            {
                ListRow row = { 0 };
                PackageEntry entry = loadedPkg.entries[i];

                row.elementCount = 4;
                row.elementWidth = (float[4]){0.3, 0.3, 0.3, 0.1};
                row.elementText = (const char*[4]){
                    TextFormat("%#X (%s)", entry.type, PackageEntryTypeToString(entry.type)),
                    TextFormat("%#X", entry.instance),
                    TextFormat("%#X", entry.group),
                    entry.compressed ? "YES" : "NO"};
                
                if (names[i])
                {
                    row.elementText[1] = TextFormat("%#X (%s)", entry.instance, names[i]);
                }

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
                4, (float[4]){0.3, 0.3, 0.3, 0.1},
                (const char *[4]){"TYPE", "INSTANCE", "GROUP", "COMPRESSED?"}
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
                    DrawPackageEntry(entry, nameList);
                }

                if (GuiButton((Rectangle){132, 0, 100, 24}, "Export Entry"))
                {
                    fileDialogReason = EXPORT_PACKAGE_ENTRY;
                    fileDialogState.saveFileMode = true;
                    fileDialogState.windowActive = true;
                }

                if (GuiButton((Rectangle){232, 0, 100, 24}, "Overwrite Entry"))
                {
                    fileDialogReason = IMPORT_FILE_OVERWRITE;
                    fileDialogState.windowActive = true;
                }

                if (entry.compressed && GuiButton((Rectangle){332, 0, 100, 24}, "Export Compressed"))
                {
                    fileDialogReason = EXPORT_PACKAGE_ENTRY_COMPRESSED;
                    fileDialogState.saveFileMode = true;
                    fileDialogState.windowActive = true;
                }
                
            }

            if (GuiButton((Rectangle){0, 0, 24, 24}, "#42#"))
            {
                findDialogState.windowActive = true;
            }
        }
        else
        {
            DrawText("Drop a .PACKAGE file to start", GetScreenWidth() / 2 - MeasureText("Drop a .PACKAGE file to start", 20)/2, GetScreenHeight() / 2 - 10, 20, GRAY);
        }

        GuiUnlock();

        GuiWindowFileDialog(&fileDialogState);
        GuiWindowFindDialog(&findDialogState);

        EndDrawing();
    }

    return 0;
}
