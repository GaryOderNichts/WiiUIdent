#include "MenuScreen.hpp"
#include "Gfx.hpp"
#include "AboutScreen.hpp"
#include "GeneralScreen.hpp"
#include "StorageScreen.hpp"
#include "SubmitScreen.hpp"

#include <vector>

MenuScreen::MenuScreen()
 : entries({
        { MENU_ID_GENERAL, { 0xf085, "General System Information" }},
        { MENU_ID_STORAGE, { 0xf7c2, "Storage Information" }},
        { MENU_ID_SUBMIT,  { 0xf0ee, "Submit System Information" }},
        // { MENU_ID_TITLE, { 0xf022, "Title Information" }},
        { MENU_ID_ABOUT,   { 0xf05a, "About WiiUIdent" }},
        // { MENU_ID_EXIT, { 0xf057, "Exit" }},
    })
{

}

MenuScreen::~MenuScreen()
{
}

void MenuScreen::Draw()
{
    if (subscreen) {
        subscreen->Draw();
        return;
    }

    DrawTopBar(nullptr);

    // draw entries
    for (MenuID id = MENU_ID_MIN; id <= MENU_ID_MAX; id = static_cast<MenuID>(id + 1)) {
        int yOff = 75 + static_cast<int>(id) * 150;
        Gfx::DrawRectFilled(0, yOff, Gfx::SCREEN_WIDTH, 150, Gfx::COLOR_ALT_BACKGROUND);
        Gfx::DrawIcon(68, yOff + 150 / 2, 60, Gfx::COLOR_TEXT, entries[id].icon);
        Gfx::Print(128 + 8, yOff + 150 / 2, 60, Gfx::COLOR_TEXT, entries[id].name, Gfx::ALIGN_VERTICAL);

        if (id == selected) {
            Gfx::DrawRect(0, yOff, Gfx::SCREEN_WIDTH, 150, 8, Gfx::COLOR_HIGHLIGHTED);
        }
    }

    DrawBottomBar("\ue07d Navigate", "\ue044 Exit", "\ue000 Select");
}

bool MenuScreen::Update(VPADStatus& input)
{
    if (subscreen) {
        if (!subscreen->Update(input)) {
            // subscreen wants to exit
            subscreen.reset();
        }
        return true;
    }

    if (input.trigger & VPAD_BUTTON_DOWN) {
        if (selected < MENU_ID_MAX) {
            selected = static_cast<MenuID>(selected + 1);
        }
    } else if (input.trigger & VPAD_BUTTON_UP) {
        if (selected > MENU_ID_MIN) {
            selected = static_cast<MenuID>(selected - 1);
        }
    }

    if (input.trigger & VPAD_BUTTON_A) {
        switch (selected) {
        case MENU_ID_GENERAL:
            subscreen = std::make_unique<GeneralScreen>();
            break;
        case MENU_ID_STORAGE:
            subscreen = std::make_unique<StorageScreen>();
            break;
        case MENU_ID_SUBMIT:
            subscreen = std::make_unique<SubmitScreen>();
            break;
        case MENU_ID_ABOUT:
            subscreen = std::make_unique<AboutScreen>();
            break;
        }
    }

    return true;
}
