#ifndef _EDITOR_
#define _EDITOR_

#include <string>
#include <vector>
#include <map>
#include <algorithm>


extern "C" {
    #include <filetypes/package.h>
    #include "../raygui.h"
    #include "../gui_window_file_dialog.h"
}

// global UI padding. really it's more of a scale
#define PADDING 20 // px

// ListView
struct ListRow {
    struct ListRowItem {
        float width;
        std::string text;
    };

    std::vector<ListRowItem> items;
};

bool GuiListRow(Rectangle bounds, ListRow row, bool selected, bool canSelect);

// first argument: index of item in list. -1 = header
// second argument: argument specified in call to GuiScrollingListPanel
typedef ListRow (*GenListRowCallback) (int, void *);

struct ScrollingListPanelData {
    Vector2 scroll;
    Rectangle view;

    int selected;
};

void GuiScrollingListPanel(Rectangle bounds, const char *title, int count,
    GenListRowCallback callback, void *callbackArg,
    ScrollingListPanelData *data); //TODO make this into a struct. 8 args is too much and I actually want to add even more to this function

// PackageLoader
class PackageLoader {

public:
    PackageLoader();
    void LoadPackage(const char *packageFilename);
    bool HasLoadedPackage();
    bool IsLoadingPackage();
    void Tick();
    std::vector<PackageEntry *> GetEntries(unsigned int type);
    PackageEntry *FindInstance(unsigned int instance);
    std::vector<unsigned int> GetTypes();

private:
    std::map<unsigned int, std::vector<PackageEntry *>> mEntries;

    bool mHasLoadedPkg;
    bool mIsLoadingPackage;
    Package mLoadedPkg;

    LoadPackageFileAsyncArgs mPackageLoadState;

    void PopulateEntries();
};

// DropdownActionList

struct DropdownAction {

    typedef void (*DropdownActionCallback) (void *);

    std::string title;
    std::string tooltip;
    DropdownActionCallback callback;
};

void GuiDropdownActionList(Rectangle bounds, std::string title, std::vector<DropdownAction> actions,
    bool *editMode, void *callbackArg);

// FileDialog
class FileDialog {
public:
    enum FileDialogMode {
        kNone,

        kSaveMode = 0x80,
        kExportPackageEntry
    };

    FileDialog();
    void Activate(FileDialogMode mode);
    void Deactivate();
    void Draw();

    bool IsFileSelected();
    bool IsActive();
    
    const char *GetSelectedFileName();
    FileDialogMode GetMode();

private:
    GuiWindowFileDialogState mState;
    FileDialogMode mMode;
};

#endif
