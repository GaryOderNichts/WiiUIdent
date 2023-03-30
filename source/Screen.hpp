#pragma once

#include <utility>
#include <vector>
#include <string>
#include <vpad/input.h>

class Screen
{
public:
    Screen() = default;
    virtual ~Screen() = default;

    virtual void Draw() = 0;

    virtual bool Update(VPADStatus& input) = 0;

protected:
    void DrawTopBar(const char* name);

    void DrawBottomBar(const char* leftHint, const char* centerHint, const char* rightHint);

    int DrawHeader(int x, int y, int w, uint16_t icon, const char* text);

    struct ScreenListElement
    {
        ScreenListElement(std::string string, bool monospace = false) 
            : string(string), monospace(monospace) {}
        ScreenListElement(const char* string, bool monospace = false)
            : string(string), monospace(monospace) {}

        std::string string;
        bool monospace;
    };
    using ScreenList = std::vector<std::pair<std::string, ScreenListElement>>;

    int DrawList(int x, int y, int w, ScreenList items);

private:
};
