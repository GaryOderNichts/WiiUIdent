#pragma once

#include "Screen.hpp"
#include "Gfx.hpp"
#include <memory>

class MainScreen : public Screen
{
public:
    MainScreen() = default;
    virtual ~MainScreen();

    void Draw();

    bool Update(VPADStatus& input);

protected:
    void DrawStatus(std::string status, SDL_Color color = Gfx::COLOR_TEXT);

private:
    enum {
        STATE_INIT,
        STATE_INIT_MOCHA,
        STATE_INIT_OTP,
        STATE_INIT_SEEPROM,
        STATE_INIT_MEMORY_DEVICE,
        STATE_LOAD_MENU,
        STATE_IN_MENU,
    } state = STATE_INIT;
    bool stateFailure = false;

    std::unique_ptr<Screen> menuScreen;
};
