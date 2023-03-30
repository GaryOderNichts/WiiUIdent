#pragma once

#include "Screen.hpp"
#include <memory>
#include <map>

class MenuScreen : public Screen
{
public:
    MenuScreen();
    virtual ~MenuScreen();

    void Draw();

    bool Update(VPADStatus& input);

private:
    std::unique_ptr<Screen> subscreen;

    enum MenuID {
        MENU_ID_GENERAL,
        MENU_ID_STORAGE,
        MENU_ID_SUBMIT,
        MENU_ID_ABOUT,

        MENU_ID_MIN = MENU_ID_GENERAL,
        MENU_ID_MAX = MENU_ID_ABOUT,
    };

    struct MenuEntry {
        uint16_t icon;
        const char* name;
    };
    std::map<MenuID, MenuEntry> entries;
    MenuID selected = MENU_ID_MIN;
};
