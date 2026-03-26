#include <raylib.h>
#include <rlWebKit.h>
#include <raymath.h>

#include <cstdlib>
#include <cstring>
extern "C" {
#include "../raygui.h"
#include "style.h"
#include <filetypes/package.h>
#include <filetypes/prop.h>
#include <threadpool.h>

}

#include "Editor.hpp"

#define PADDING 20 // px

struct EditorState {
    bool hasLoadedPkg;
    bool isLoadingPackage;
    Package loadedPkg;
    PropertyNameList propNameList;

    std::vector<PackageEntry*> propEntries;
    std::vector<const char *> propEntryNames;
    int selectedPropEntry;

    const char *packageToLoad;
    LoadPackageFileAsyncArgs packageLoadState;

    Vector2 propScroll;
    Rectangle propView;
    int selectedPropVal;

    Vector2 dataScroll;
    Rectangle dataView;

    Vector2 valScroll;
    Rectangle valView;

    std::string statusText;
};

void SetLoadedPackage(EditorState &state, const char *packageFilename)
{
    FILE *f = fopen(packageFilename, "rb");

    if (!f)
    {
        perror(packageFilename);
        return;
    }

    state.selectedPropEntry = -1;
    if (state.hasLoadedPkg)
    {
        UnloadPackageFile(state.loadedPkg);
        state.propEntries.clear();
        state.propEntryNames.clear();
    }

    state.hasLoadedPkg = false;

    state.packageLoadState = {
        .f = f,
        .pkg = &state.loadedPkg,
        .done = false,
    };

    LoadPackageFileAsync(&state.packageLoadState);
    state.isLoadingPackage = true;

    state.statusText = std::string(TextFormat("Loaded: %s", packageFilename));
}

const char *FindPropNameForId(PropertyNameList nameList, unsigned int id)
{
    for (int i = 0; i < nameList.propCount; i++)
    {
        if (nameList.propIds[i] == id)
            return nameList.propNames[i];
    }

    return NULL;
}

void PopulatePropEntries(EditorState &state)
{
    for (int i = 0; i < state.loadedPkg.entryCount; i++)
    {
        PackageEntry *entry = &state.loadedPkg.entries[i];
        if (entry->type != PKGENTRY_PROP)
            continue;
        
        const char *name = FindPropNameForId(state.propNameList, entry->instance);        

        entry->data.propData = LoadPropData(entry->dataRaw, entry->dataRawSize);

        state.propEntries.push_back(entry);
        state.propEntryNames.push_back(name);
    }
}

void Update(EditorState &state)
{
    if (state.packageToLoad)
    {
        SetLoadedPackage(state, state.packageToLoad);
        state.packageToLoad = NULL;
    }
    else if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();
        if (IsFileExtension(droppedFiles.paths[0], ".package"))
            SetLoadedPackage(state, droppedFiles.paths[0]);
    }

    if (state.isLoadingPackage)
    {
        state.isLoadingPackage = !state.packageLoadState.done;
        if (!state.isLoadingPackage)
        {
            // done loading package
            state.hasLoadedPkg = true;
            PopulatePropEntries(state);
            fclose(state.packageLoadState.f);
        }
    }

    if (state.hasLoadedPkg)
    {
        if (IsKeyPressed(KEY_DOWN))
        {
            state.selectedPropEntry = Wrap(state.selectedPropEntry + 1, 0, state.propEntries.size());
            state.selectedPropVal = 0;
        }
        if (IsKeyPressed(KEY_UP))
        {
            state.selectedPropEntry = Wrap(state.selectedPropEntry - 1, 0, state.propEntries.size());
            state.selectedPropVal = 0;
        }
    }
}

ListRow GenListRowForPackageEntry(EditorState state, int i, PackageEntry entry)
{
    ListRow row;

    ListRow::ListRowItem nameItem;
    nameItem.width = 0.7f;
    if (state.propEntryNames.at(i))
    {
        nameItem.text = std::string(TextFormat("%#X (%s)", entry.instance, state.propEntryNames[i]));
    }
    else
    {
        nameItem.text = std::string(TextFormat("%#X", entry.instance));
    }

    ListRow::ListRowItem groupItem;
    groupItem.text = std::string(TextFormat("%#X", entry.group));
    groupItem.width = 0.3f; // 0.5

    row.items = {nameItem, groupItem};


    return row;
}

const char *PropTypeToString(unsigned int type)
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

const char *PropVarToString(PropVariable var, int i)
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

void GoToInstance(EditorState &state, unsigned int id)
{
    for (int i = 0; i < state.propEntries.size(); i++)
    {
        PackageEntry *entry = state.propEntries[i];
        if (entry->instance != id) continue;

        state.selectedPropEntry = i;
        break;
    }
}

void DrawPropValueMenu(EditorState &state, unsigned int id, PropVariable var)
{
    GuiScrollPanel((Rectangle){
        .x = PADDING*3 + (GetScreenWidth() - 2*PADDING)/2,
        .y = PADDING + GetScreenHeight()/2 - PADDING *2,
        .width = GetScreenWidth()/2 - 3*PADDING,
        .height = GetScreenHeight()/2 - PADDING *2
    }, TextFormat("Values of %#X!%#X", id, var.identifier), (Rectangle){
        .x = PADDING*3 + (GetScreenWidth() - 2*PADDING)/2,
        .y = PADDING*2 + GetScreenHeight()/2 - PADDING *2,
        .width = (GetScreenWidth() - 2*PADDING)/2,
        .height = var.count*PADDING + PADDING*4
    }, &state.valScroll, &state.valView);

    BeginScissorMode(state.valView.x, state.valView.y, state.valView.width, state.valView.height);
    
    for (int i = 0; i < var.count; i++)
    {
        ListRow row = {
            {
                {0.2f, TextFormat("%d", i)},
                {0.8f, PropVarToString(var, i)},
            }
        };
        bool selected = false;

        bool pressed = GuiListRow((Rectangle){
            .x = PADDING*3 + (GetScreenWidth() - 2*PADDING)/2,
            .y = PADDING*2 + GetScreenHeight()/2 - PADDING *2 + PADDING*i + state.valScroll.y + PADDING,
            .width = state.valView.width,
            .height = PADDING,
        }, row, selected, true);

        if (pressed && var.type == PROPVAR_KEYS)
        {
            GoToInstance(state, var.values[i].keys.file);
        }
    }

    GuiDummyRec((Rectangle){
        PADDING*3 + (GetScreenWidth() - 2*PADDING)/2, PADDING*2 + GetScreenHeight()/2 - PADDING *2,
        state.valView.width,
        PADDING
    }, "");
    GuiListRow((Rectangle){
        PADDING*3 + (GetScreenWidth() - 2*PADDING)/2, PADDING*2 + GetScreenHeight()/2 - PADDING *2,
        state.valView.width,
        PADDING
    }, (ListRow){
        {
            {0.2f, "#"},
            {0.8f, "VALUE"}
        }
    }, false, false);

    EndScissorMode();
}

void DrawPropMenu(EditorState &state, PropData data, unsigned int id)
{
    GuiScrollPanel((Rectangle){
        .x = PADDING*3 + (GetScreenWidth() - 2*PADDING)/2,
        .y = PADDING,
        .width = GetScreenWidth()/2 - 3*PADDING,
        .height = GetScreenHeight()/2 - PADDING *2
    }, TextFormat("Properties of %#X", id), (Rectangle){
        .x = PADDING*3 + (GetScreenWidth() - 2*PADDING)/2,
        .y = PADDING*2,
        .width = (GetScreenWidth() - 2*PADDING)/2,
        .height = data.variableCount*PADDING + PADDING*4
    }, &state.dataScroll, &state.dataView);

    BeginScissorMode(state.dataView.x, state.dataView.y, state.dataView.width, state.dataView.height);
    
    for (int i = 0; i < data.variableCount; i++)
    {
        PropVariable var = data.variables[i];
        ListRow row = {
            {
                {0.4f, TextFormat("%#X (%s)", var.type, PropTypeToString(var.type))},
                {0.4f, TextFormat("%#X (%s)", var.identifier, FindPropNameForId(state.propNameList, var.identifier) ?: "")},
                {0.2f, TextFormat("%d", var.count)}
            }
        };
        bool selected = i == state.selectedPropVal;

        bool pressed = GuiListRow((Rectangle){
            .x = PADDING*3 + (GetScreenWidth() - 2*PADDING)/2,
            .y = PADDING*2 + PADDING*i + state.dataScroll.y + PADDING,
            .width = state.propView.width,
            .height = PADDING,
        }, row, selected, true);

        if (pressed)
        {
            state.selectedPropVal = selected ? -1 : i;
        }
    }

    GuiDummyRec((Rectangle){
        PADDING*3 + (GetScreenWidth() - 2*PADDING)/2, PADDING*2,
        state.propView.width,
        PADDING
    }, "");
    GuiListRow((Rectangle){
        PADDING*3 + (GetScreenWidth() - 2*PADDING)/2, PADDING*2,
        state.propView.width,
        PADDING
    }, (ListRow){
        {
            {0.4f, "TYPE"},
            {0.4f, "IDENTIFIER"},
            {0.2f, "#"}
        }
    }, false, false);

    EndScissorMode();

    if (state.selectedPropVal != -1)
    {
        DrawPropValueMenu(state, id, data.variables[state.selectedPropVal]);
    }
}

void DrawMainScreen(EditorState &state)
{
    GuiScrollPanel((Rectangle){
        .x = PADDING,
        .y = PADDING,
        .width = PADDING + (GetScreenWidth() - 2*PADDING)/2,
        .height = GetScreenHeight() - PADDING*2
    }, "Property List", (Rectangle){
        .x = PADDING,
        .y = PADDING*2,
        .width = PADDING + (GetScreenWidth() - 2*PADDING)/2,
        .height = state.propEntries.size()*PADDING + PADDING*4
    }, &state.propScroll, &state.propView);

    BeginScissorMode(state.propView.x, state.propView.y, state.propView.width, state.propView.height);

    for (int i = 0; i < state.propEntries.size(); i++)
    {
        PackageEntry *entry = state.propEntries[i];
        ListRow row = GenListRowForPackageEntry(state, i, *entry);
        bool selected = state.selectedPropEntry ==i;

        bool pressed = GuiListRow((Rectangle){
            .x = PADDING,
            .y = PADDING*2 + PADDING*i + state.propScroll.y + PADDING,
            .width = state.propView.width,
            .height = PADDING,
        }, row, selected, true);
    
        if (pressed)
        {
            state.selectedPropVal = -1;
            state.selectedPropEntry = selected ? -1 : i;
        }
    }

    GuiDummyRec((Rectangle){
        PADDING, PADDING*2,
        state.propView.width,
        PADDING
    }, "");
    GuiListRow((Rectangle){
        PADDING, PADDING*2,
        state.propView.width,
        PADDING
    }, (ListRow){
        {
            {0.7f, "INSTANCE"},
            {0.3f, "GROUP"}
        }
    }, false, false);

    EndScissorMode();

    if (state.selectedPropEntry != -1)
    {
        PropData data = state.propEntries[state.selectedPropEntry]->data.propData;
        if (data.corrupted)
        {
            DrawText("CORRUPTED. see corrupted/ folder", 3*GetScreenWidth()/4, GetScreenHeight()/2, 20, GRAY);
        }
        else
        {
            DrawPropMenu(state, data, state.propEntries[state.selectedPropEntry]->instance);
        }
    }

    GuiStatusBar((Rectangle){0, GetScreenHeight()-PADDING, GetScreenWidth(), PADDING}, state.statusText.c_str());
}

void Draw(EditorState &state)
{
    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    if (!state.hasLoadedPkg)
    {
        if (state.isLoadingPackage)
        {
            DrawText(TextFormat("Loading... %d left", GetThreadpoolTasksLeft()),
                GetScreenWidth() / 2 - MeasureText("Loading...", 20)/2, GetScreenHeight() / 2 - 10, 20, GRAY);
        }
        else
        {
            DrawText(TextFormat("Drop a package to start", GetThreadpoolTasksLeft()),
                GetScreenWidth() / 2 - MeasureText("Drop a package to start", 20)/2, GetScreenHeight() / 2 - 10, 20, GRAY);
        }
    }
    else
    {
        DrawMainScreen(state);
    }

    EndDrawing();
}

int main(int argc, char **argv)
{
    EditorState state;

    initWebkit(); // We must init webkit because they overrode our allocators

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "OpenSC5 Editor v2.0");

    SetTargetFPS(60);
    GuiLoadStyleDark(); //dark theme on top

    SetTryParseFilesInPackage(false);
    state.propNameList = LoadPropertyNameList("Properties.txt");
    state.hasLoadedPkg = false;
    state.packageToLoad = NULL;
    state.isLoadingPackage = false;
    state.statusText = "";

    if (argc == 2)
    {
        state.packageToLoad = argv[1];
    }

    while (!WindowShouldClose())
    {
        Update(state);
        Draw(state);
    }
}
