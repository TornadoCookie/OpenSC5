#include "Editor.hpp"

bool GuiListRow(Rectangle bounds, ListRow row, bool selected, bool canSelect)
{
    int xOff = 0;
    bool focused = false;
    bool pressed = false;

    if (bounds.y + bounds.height < 0 || bounds.y > GetScreenHeight()) return false;

    if (canSelect && CheckCollisionPointRec(GetMousePosition(), bounds) && GetMousePosition().y > 20*2 && !GuiIsLocked()) 
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

    for (int i = 0; i < row.items.size(); i++)
    {
        Rectangle buttonBounds = (Rectangle){
            bounds.x + xOff,
            bounds.y,
            bounds.width * row.items[i].width,
            bounds.height
        };
        //GuiDrawRectangle(buttonBounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), GetColor(GuiGetStyle(TEXTBOX, BORDER + GuiGetState()*3)), selected?GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)):RAYWHITE);
        GuiSetStyle(TEXTBOX, TEXT_READONLY, true);
        GuiTextBox(buttonBounds, "", 0, false);
        GuiSetStyle(TEXTBOX, TEXT_READONLY, false);
        GuiLabel(buttonBounds, row.items[i].text.c_str());
        xOff += bounds.width * row.items[i].width;
    }

    if (selected || focused || pressed)
    {
        GuiSetState(STATE_NORMAL);
    }
    return pressed;
}

void GuiScrollingListPanel(Rectangle bounds, const char *title, Vector2 *scroll,
    Rectangle *view, int count, GenListRowCallback callback, void *callbackArg,
    int *selected)
{
    GuiScrollPanel(bounds, title, (Rectangle){
        .x = bounds.x,
        .y = bounds.y + PADDING,
        .width = bounds.width,
        .height = (count + 4) * PADDING
    }, scroll, view);

    BeginScissorMode(view->x, view->y, view->width, view->height);

    for (int i = 0; i < count; i++)
    {
        ListRow row = callback(i, callbackArg);
        bool isSelected = i == *selected;

        bool pressed = GuiListRow((Rectangle){
            .x = bounds.x,
            .y = bounds.y + PADDING*2 + PADDING * i + scroll->y,
            .width = bounds.width,
            .height = PADDING
        }, row, isSelected, true);

        if (pressed)
        {
            *selected = isSelected ? -1 : i;
        }
    }

    GuiDummyRec((Rectangle){
        .x = bounds.x,
        .y = bounds.y + PADDING,
        .width = bounds.width,
        .height = PADDING
    }, "");
    GuiListRow((Rectangle){
        .x = bounds.x,
        .y = bounds.y + PADDING,
        .width = bounds.width,
        .height = PADDING
    }, callback(-1, callbackArg), false, false);

    EndScissorMode();
}
