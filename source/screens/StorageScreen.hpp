#pragma once

#include "Screen.hpp"

class StorageScreen : public Screen
{
public:
    StorageScreen();
    virtual ~StorageScreen();

    void Draw();

    bool Update(VPADStatus& input);

private:
    ScreenList mlcList;
    ScreenList sdList;
};
