#ifndef _EDITOR_
#define _EDITOR_

#include <string>
#include <vector>

#include "../raygui.h"

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

void GuiScrollingListPanel(Rectangle bounds, const char *title, Vector2 *scroll,
    Rectangle *view, int count, GenListRowCallback callback, void *callbackArg,
    int *selected); //TODO make this into a struct. 8 args is too much and I actually want to add even more to this function

#endif
