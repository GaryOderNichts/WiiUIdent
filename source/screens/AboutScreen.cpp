#include "AboutScreen.hpp"

AboutScreen::AboutScreen()
{
    creditList.push_back({"Developers:", "GaryOderNichts"});
    creditList.push_back({"",            "GerbilSoft"});

    fontList.push_back({"Main Font:", "Wii U System Font"});
    fontList.push_back({"Icon Font:", "FontAwesome"});
    fontList.push_back({"Monospace Font:", "Terminus Font"});

    linkList.push_back({"GitHub:", ""});
    linkList.push_back({"", {"github.com/GaryOderNichts/WiiUIdent", true}});
    linkList.push_back({"System Database:", {DATABASE_URL, true}});
}

AboutScreen::~AboutScreen()
{
}

void AboutScreen::Draw()
{
    DrawTopBar("About");

    int yOff = 128;
    yOff = DrawHeader(32, yOff, 896, 0xf121, "Credits");
    yOff = DrawList(32, yOff, 896, creditList);
    yOff = DrawHeader(32, yOff, 896, 0xf031, "Fonts");
    yOff = DrawList(32, yOff, 896, fontList);

    yOff = 128;
    yOff = DrawHeader(992, yOff, 896, 0xf08e, "Links");
    yOff = DrawList(992, yOff, 896, linkList);

    DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back");
}

bool AboutScreen::Update(VPADStatus& input)
{
    if (input.trigger & VPAD_BUTTON_B) {
        return false;
    }

    return true;
}
