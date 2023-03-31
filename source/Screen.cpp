#include "Screen.hpp"
#include "Gfx.hpp"

void Screen::DrawTopBar(const char* name)
{
    // draw top bar
    Gfx::DrawRectFilled(0, 0, Gfx::SCREEN_WIDTH, 75, Gfx::COLOR_BARS);

    // draw top bar content
    Gfx::DrawIcon(32, 75 / 2, 60, Gfx::COLOR_TEXT, 0xf002, Gfx::ALIGN_VERTICAL);
    Gfx::Print(128, 75 / 2, 60, Gfx::COLOR_TEXT, "WiiUIdent", Gfx::ALIGN_VERTICAL);
    Gfx::Print(Gfx::GetTextWidth(60, "WiiUIdent") + 128 + 16, 75 / 2 + 5, 50, Gfx::COLOR_ALT_TEXT, "v" APP_VERSION, Gfx::ALIGN_VERTICAL);
    if (name)
        Gfx::Print(Gfx::SCREEN_WIDTH - 32, 75 / 2, 50, Gfx::COLOR_ALT_TEXT, name, Gfx::ALIGN_VERTICAL | Gfx::ALIGN_RIGHT);
}

void Screen::DrawBottomBar(const char* leftHint, const char* centerHint, const char* rightHint)
{
    // draw bottom bar
    Gfx::DrawRectFilled(0, Gfx::SCREEN_HEIGHT - 75, Gfx::SCREEN_WIDTH, 75, Gfx::COLOR_BARS);

    // draw bottom bar content
    if (leftHint)
        Gfx::Print(32, Gfx::SCREEN_HEIGHT - 75 / 2, 50, Gfx::COLOR_TEXT, leftHint, Gfx::ALIGN_VERTICAL);
    if (centerHint)
        Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT - 75 / 2, 50, Gfx::COLOR_TEXT, centerHint, Gfx::ALIGN_CENTER);
    if (rightHint)
        Gfx::Print(Gfx::SCREEN_WIDTH - 32,  Gfx::SCREEN_HEIGHT - 78 / 2, 50, Gfx::COLOR_TEXT, rightHint, Gfx::ALIGN_VERTICAL | Gfx::ALIGN_RIGHT);
}

int Screen::DrawHeader(int x, int y, int w, uint16_t icon, const char* text)
{
    const int iconWidth = Gfx::GetIconWidth(50, icon);
    const int width = iconWidth + 32 + Gfx::GetTextWidth(50, text);
    const int xStart = x + (w / 2) - (width / 2);

    Gfx::DrawIcon(xStart, y, 50, Gfx::COLOR_TEXT, icon, Gfx::ALIGN_VERTICAL);
    Gfx::Print(xStart + iconWidth + 32, y, 50, Gfx::COLOR_TEXT, text, Gfx::ALIGN_VERTICAL);
    Gfx::DrawRectFilled(x, y + 32, w, 4, Gfx::COLOR_ACCENT);

    return y + 64;
}

int Screen::DrawList(int x, int y, int w, ScreenList items)
{
    int yOff = y;
    for (const auto& item : items) {
        Gfx::Print(x + 16, yOff, 40, Gfx::COLOR_TEXT, item.first, Gfx::ALIGN_VERTICAL);
        Gfx::Print(x + w - 16, yOff, 40, Gfx::COLOR_TEXT, item.second.string, Gfx::ALIGN_VERTICAL | Gfx::ALIGN_RIGHT, item.second.monospace);
        yOff += std::max(Gfx::GetTextHeight(40, item.first), Gfx::GetTextHeight(40, item.second.string, item.second.monospace));
    }

    return yOff + 32;
}
