#include <raylib.h>
#include <rlWebKit.h>
#include <raymath.h>

#include <cstdlib>
#include <cstring>
#include <algorithm>

#define RAYGUI_MALLOC malloc

extern "C" {
#include "../raygui.h"
#include "style.h"
#include <filetypes/package.h>
#include <filetypes/prop.h>
#include <threadpool.h>

}

#include "Editor.hpp"

struct EditorState {
    Package loadedPkg;
    PropertyNameList propNameList;

    int selectedType;
    std::map<unsigned int, std::vector<PackageEntry *>> entries;

    PackageLoader packageLoader;

    std::vector<const char *> propEntryNames;

    const char *packageToLoad;
    
    ScrollingListPanelData mainListData;
    ScrollingListPanelData propListData;
    ScrollingListPanelData propValData;

    std::string statusText;
    bool typeDropdownEditMode;
    bool packageOptionsDropdownEditMode;
    bool packageEntryOptionsDropdownEditMode;

    FileDialog fileDialog;

    Vector2 entryScroll;
    Rectangle entryView;
};

void SetLoadedPackage(EditorState &state, const char *packageFilename)
{
    state.mainListData.selected = -1;
    state.statusText = std::string(TextFormat("Loaded: %s", packageFilename));
    
    if (state.packageLoader.HasLoadedPackage())
    {
        // a package was loaded beforehand
        state.propEntryNames.clear();
    }

    state.packageLoader.LoadPackage(packageFilename);
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

void ProcessFileDialogSelection(EditorState &state)
{
    switch (state.fileDialog.GetMode())
    {
        case FileDialog::FileDialogMode::kExportPackageEntry:
        {
            std::vector<PackageEntry *> entries = state.packageLoader.GetEntries(state.selectedType);
            PackageEntry entry = *entries[state.mainListData.selected];
            ExportPackageEntry(entry, state.fileDialog.GetSelectedFileName());
            state.statusText = "Successfully Exported " + std::string(state.fileDialog.GetSelectedFileName()) + ".";
        } break;
        default:
        {
            TRACELOG(LOG_WARNING, "invalid state %d", state.fileDialog.GetMode());
        } break;
    }
    
    state.fileDialog.Deactivate();
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

    state.packageLoader.Tick();

    if (state.packageLoader.HasLoadedPackage())
    {
        std::vector<PackageEntry*> entries = state.packageLoader.GetEntries(state.selectedType);
        if (IsKeyPressed(KEY_DOWN))
        {
            state.mainListData.selected = Wrap(state.mainListData.selected + 1, 0, entries.size());
            state.propListData.selected = -1;
        }
        if (IsKeyPressed(KEY_UP))
        {
            state.mainListData.selected = Wrap(state.mainListData.selected - 1, 0, entries.size());
            state.propListData.selected = -1;
        }
    }

    if (state.fileDialog.IsFileSelected())
    {
        ProcessFileDialogSelection(state);
    }
}

ListRow GenListRowForPackageEntry(EditorState state, int i, PackageEntry entry)
{
    ListRow row;

    ListRow::ListRowItem nameItem;
    nameItem.width = 0.7f;
    nameItem.text = std::string(TextFormat("%#X (%s)", entry.instance, FindPropNameForId(state.propNameList, entry.instance)));


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
        case PROPVAR_COLRGB: return TextFormat("%f, %f, %f, %f", var.values[i].colorRGB.r*255, var.values[i].colorRGB.g*255, var.values[i].colorRGB.b*255);
        case PROPVAR_CRGBA: return TextFormat("%f, %f, %f, %f", var.values[i].colorRGBA.r*255, var.values[i].colorRGBA.g*255, var.values[i].colorRGBA.b*255, var.values[i].colorRGBA.a*255);
        case PROPVAR_BBOX: return TextFormat("min {%f, %f, %f}, max {%f, %f %f}", var.values[i].bbox.min.x, var.values[i].bbox.min.y, var.values[i].bbox.min.z, 
                                             var.values[i].bbox.max.x, var.values[i].bbox.max.y, var.values[i].bbox.max.z);
        default: return "Unable to read type";
    }
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
        case PKGENTRY_SHDR: return "SHDR";
        case PKGENTRY_CUR: return "CUR";
        case PKGENTRY_MAP8: return "MAP8";
        case PKGENTRY_MAP2: return "MAP2";
        default: { printf("%#X\n", type); return "UNKN"; }
    }
}

void GoToInstance(EditorState &state, unsigned int id)
{
    PackageEntry *entry = state.packageLoader.FindInstance(id);

    if (!entry)
    {
        state.statusText = "Entry not found.";
        return;
    }

    state.selectedType = entry->type;
    std::vector<PackageEntry *> entries = state.packageLoader.GetEntries(entry->type);

    auto it = std::find(entries.begin(), entries.end(), entry);
    
    state.mainListData.selected = std::distance(entries.begin(), it);
}

static ListRow GenPropValueListRow(int i, void *data)
{
    if (i == -1)
    {
        return (ListRow){
            {
                {0.2f, "#"},
                {0.8f, "VALUE"}
            }
        };
    }

    PropVariable *pVar = static_cast<PropVariable *>(data);

    ListRow row = {
        {
            {0.2f, TextFormat("%d", i)},
            {0.8f, PropVarToString(*pVar, i)},
        }
    };

    return row;
}

void DrawPropValueMenu(EditorState &state, unsigned int id, PropVariable var)
{
    GuiScrollingListPanel((Rectangle){
        .x = GetScreenWidth()/2,
        .y = PADDING*2 + GetScreenHeight()/2,
        .width = GetScreenWidth()/2 - 2*PADDING,
        .height = GetScreenHeight()/2 - 2*PADDING
    }, TextFormat("Properties of %#X!%#X", id, var.identifier), 
        var.count, GenPropValueListRow, &var, &state.propValData);

    int selected = state.propValData.selected;
    if (selected != -1 && var.type == PROPVAR_KEYS)
    {
        GoToInstance(state, var.values[selected].keys.file);
    }
}

struct PropMenuData {
    EditorState *state;
    PropData data;
};

static ListRow GenPropMenuListRow(int i, void *data)
{
    if (i == -1)
    {
        return (ListRow){
        {
            {0.4f, "TYPE"},
            {0.4f, "IDENTIFIER"},
            {0.2f, "#"}
        }};
    }

    PropMenuData *menuData = static_cast<PropMenuData*>(data);
    PropVariable var = menuData->data.variables[i];
    EditorState *state = menuData->state;

    ListRow row = {
        {
            {0.4f, TextFormat("%#X (%s)", var.type, PropTypeToString(var.type))},
            {0.4f, TextFormat("%#X (%s)", var.identifier, FindPropNameForId(state->propNameList, var.identifier) ?: "")},
            {0.2f, TextFormat("%d", var.count)}
        }
    };

    return row;
}

void DrawPropMenu(EditorState &state, PropData data, unsigned int id)
{
    PropMenuData menuData = {
        .state = &state,
        .data = data
    };

    int prevSelected = state.propListData.selected;

    GuiScrollingListPanel((Rectangle){
        .x = GetScreenWidth()/2,
        .y = PADDING,
        .width = GetScreenWidth()/2 - 2*PADDING,
        .height = GetScreenHeight()/2 - 2*PADDING
    }, TextFormat("Properties of %#X", id), 
        data.variableCount, GenPropMenuListRow, &menuData, &state.propListData);

    if (state.propListData.selected != -1)
    {
        if (state.propListData.selected != prevSelected)
        {
            state.propValData.selected = -1;
        }
        DrawPropValueMenu(state, id, data.variables[state.propListData.selected]);
    }
}

static ListRow GenPropListRow(int i, void *pState)
{
    if (i == -1)
    {
        return (ListRow){{
            {0.7f, "INSTANCE"},
            {0.3f, "GROUP"}
        }};
    }

    EditorState *state = static_cast<EditorState*>(pState);
    PackageEntry *entry = state->packageLoader.GetEntries(state->selectedType)[i];
    ListRow row = GenListRowForPackageEntry(*state, i, *entry);

    return row;
}

static std::string GenTypeListStr(std::vector<unsigned int> types)
{
    std::string out;

    for (unsigned int type : types)
    {
        const char *typeStr = PackageEntryTypeToString(type);
        out += typeStr;
        out += ";";
    }

    return out;
}

int GetTextHeight(const char *text, int textSize)
{
    return MeasureTextEx(GuiGetFont(), text, textSize, 1.0f).y;
}

void DrawTextMenu(EditorState &state, const char *text)
{
    GuiScrollPanel((Rectangle){
        .x = GetScreenWidth()/2,
        .y = PADDING,
        .width = GetScreenWidth()/2 - 2*PADDING,
        .height = GetScreenHeight() - 2*PADDING
    }, "Source", (Rectangle)
    {
        .x = GetScreenWidth()/2,
        .y = PADDING*2,
        .width = GetScreenWidth()/2 - 2*PADDING,
        .height = GetTextHeight(text, 10)
    }, &state.entryScroll, &state.entryView);

    BeginScissorMode(state.entryView.x, state.entryView.y, state.entryView.width, state.entryView.height);

    GuiTextBox((Rectangle){
        .x = GetScreenWidth()/2,
        .y = PADDING + state.entryScroll.y,
        .width = GetScreenWidth()/2 - 2*PADDING,
        .height = GetTextHeight(text, 10)
    }, (char*)text, 10, false);

    EndScissorMode();

}

void DrawPackageEntry(EditorState &state, PackageEntry *entry)
{
    bool corrupted = true;
    switch (entry->type)
    {
        case PKGENTRY_PROP:
        {
            PropData data = entry->data.propData;
            corrupted = data.corrupted;
            if (!corrupted) DrawPropMenu(state, data, entry->instance);
        } break;
        case PKGENTRY_ER2:
        case PKGENTRY_HTML:
        case PKGENTRY_CSS:
        case PKGENTRY_JSN8:
        case PKGENTRY_SCPT:
        case PKGENTRY_TEXT:
        case PKGENTRY_JSON:
        {
            DrawTextMenu(state, entry->data.scriptSource);
        } break;
    }

    if (corrupted)
    {
        DrawText("CORRUPTED. see corrupted/ folder", 5*GetScreenWidth()/8, GetScreenHeight()/2, 20, GRAY);
    }
}

static void Action_ExportPackageEntry(void *arg)
{
    EditorState *state = (EditorState *)arg;
    state->fileDialog.Activate(FileDialog::FileDialogMode::kExportPackageEntry);
}

void DrawMainScreen(EditorState &state)
{
    int prevSelected = state.mainListData.selected;
    std::vector<PackageEntry *> entries = state.packageLoader.GetEntries(state.selectedType);

    if (state.fileDialog.IsActive()) GuiLock();

    if (state.typeDropdownEditMode || state.packageOptionsDropdownEditMode || state.packageEntryOptionsDropdownEditMode) GuiLock();

    GuiScrollingListPanel((Rectangle){
        .x = PADDING,
        .y = PADDING,
        .width = GetScreenWidth() / 2 - PADDING * 2,
        .height = GetScreenHeight() - PADDING * 2
    }, "Property List",
        entries.size(), GenPropListRow, &state, &state.mainListData);

    if (state.mainListData.selected != -1)
    {
        if (state.mainListData.selected != prevSelected)
        {
            state.propListData.selected = -1;
        }

        DrawPackageEntry(state, entries[state.mainListData.selected]);
    }

    if (state.typeDropdownEditMode || state.packageOptionsDropdownEditMode || state.packageEntryOptionsDropdownEditMode) GuiUnlock();

    if (state.fileDialog.IsActive()) GuiLock();

    GuiDropdownActionList((Rectangle){
        .x = PADDING + 100,
        .y = 0,
        .width = 100,
        .height = PADDING
    }, "Package", {
    }, &state.packageOptionsDropdownEditMode, &state);

    if (state.mainListData.selected != -1)
    {
        GuiDropdownActionList((Rectangle){
            .x = GetScreenWidth()/2,
            .y = 0,
            .width = 100,
            .height = PADDING
        }, "Package Entry", {
            {"Export", "Export the package entry to a file.", Action_ExportPackageEntry},
        }, &state.packageEntryOptionsDropdownEditMode, &state);
    }

    std::vector<unsigned int> types = state.packageLoader.GetTypes();
    std::string typeStr = GenTypeListStr(types);
    int selectedTypeI = std::distance(types.begin(), std::find(types.begin(), types.end(), state.selectedType));
    if (GuiDropdownBox((Rectangle){
        .x = PADDING,
        .y = 0,
        .width = 100,
        .height = PADDING
    }, typeStr.c_str(), &selectedTypeI, state.typeDropdownEditMode))
    {
        state.typeDropdownEditMode = !state.typeDropdownEditMode;
    }
    unsigned int prevType = state.selectedType;
    state.selectedType = types[selectedTypeI];

    if (state.selectedType != prevType)
    {
        state.mainListData.selected = -1;
    }

    GuiStatusBar((Rectangle){0, GetScreenHeight()-PADDING, GetScreenWidth(), PADDING}, state.statusText.c_str());

    if (state.fileDialog.IsActive()) GuiUnlock();

    state.fileDialog.Draw();
}

void Draw(EditorState &state)
{
    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    if (!state.packageLoader.HasLoadedPackage())
    {
        if (state.packageLoader.IsLoadingPackage())
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

    SetTargetFPS(30);
    GuiLoadStyleDark(); //dark theme on top

    SetTryParseFilesInPackage(false);
    state.propNameList = LoadPropertyNameList("Properties.txt");
    state.packageToLoad = NULL;
    state.statusText = "";
    state.selectedType = PKGENTRY_PROP;
    state.typeDropdownEditMode = false;
    state.packageOptionsDropdownEditMode = false;
    state.packageEntryOptionsDropdownEditMode = false;

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
