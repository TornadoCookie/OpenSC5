#ifndef _EDITOR_
#define _EDITOR_

#include <string>
#include <vector>

#include "../raygui.h"

// ListView
struct ListRow {
    struct ListRowItem {
        float width;
        std::string text;
    };

    std::vector<ListRowItem> items;
};

bool GuiListRow(Rectangle bounds, ListRow row, bool selected, bool canSelect);

#endif
