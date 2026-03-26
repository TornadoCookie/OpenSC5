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
