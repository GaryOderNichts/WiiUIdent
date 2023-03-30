#pragma once

#include "Screen.hpp"

class GeneralScreen : public Screen
{
public:
    GeneralScreen();
    virtual ~GeneralScreen();

    void Draw();

    bool Update(VPADStatus& input);

private:
    ScreenList sysIdentList;
    ScreenList hardwareList;
    ScreenList regionList;
    ScreenList versionList;
};
