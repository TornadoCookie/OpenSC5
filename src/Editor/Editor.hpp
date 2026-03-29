#ifndef _EDITOR_
#define _EDITOR_

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "../raygui.h"

extern "C" {
    #include <filetypes/package.h>
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

#endif
