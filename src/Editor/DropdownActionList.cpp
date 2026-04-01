#include "Editor.hpp"

void GuiDropdownActionList(Rectangle bounds, std::string title, std::vector<DropdownAction> actions, bool *editMode, void *callbackArg)
{
    std::string text = title;

    for (int i = 0; i < actions.size(); i++)
    {
        DropdownAction action = actions[i];
        text += ";" + action.title;
    }

    int selectedAction = 0;

    if (GuiDropdownBox(bounds, text.c_str(), &selectedAction, *editMode))
    {
        *editMode = !*editMode;
    }

    if (selectedAction != 0 && actions[selectedAction - 1].callback)
    {
        actions[selectedAction - 1].callback(callbackArg);
    }
}
