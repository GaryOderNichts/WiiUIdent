#include "MainScreen.hpp"
#include "MenuScreen.hpp"
#include "Gfx.hpp"
#include "system/OTP.hpp"
#include "system/SEEPROM.hpp"
#include "system/MemoryDevice.hpp"

#include <vector>

#include <mocha/mocha.h>

MainScreen::~MainScreen()
{
    if (state > STATE_INIT_MOCHA) {
        Mocha_DeInitLibrary();
    }
}

void MainScreen::Draw()
{
    Gfx::Clear(Gfx::COLOR_BACKGROUND);

    if (menuScreen) {
        menuScreen->Draw();
        return;
    }

    DrawTopBar(nullptr);

    switch (state) {
        case STATE_INIT:
            break;
        case STATE_INIT_MOCHA:
            if (stateFailure) {
                DrawStatus("Failed to initialize mocha!\nMake sure to update or install Tiramisu/Aroma.", Gfx::COLOR_ERROR);
                break;
            }

            DrawStatus("Initializing mocha...");
            break;
        case STATE_INIT_OTP:
            if (stateFailure) {
                DrawStatus("Failed to initialize OTP!", Gfx::COLOR_ERROR);
                break;
            }

            DrawStatus("Initializing OTP...");
            break;
        case STATE_INIT_SEEPROM:
            if (stateFailure) {
                DrawStatus("Failed to initialize SEEPROM!", Gfx::COLOR_ERROR);
                break;
            }

            DrawStatus("Initializing SEEPROM...");
            break;
        case STATE_INIT_MEMORY_DEVICE:
            if (stateFailure) {
                DrawStatus("Failed to initialize Memory Devices!", Gfx::COLOR_ERROR);
                break;
            }

            DrawStatus("Initializing Memory Devices...");
            break;
        case STATE_LOAD_MENU:
            DrawStatus("Loading menu...");
            break;
        case STATE_IN_MENU:
            break;
    }

    DrawBottomBar(stateFailure ? nullptr : "Please wait...", stateFailure ? "\ue044 Exit" : nullptr, nullptr);
}

bool MainScreen::Update(VPADStatus& input)
{
    if (menuScreen) {
        if (!menuScreen->Update(input)) {
            // menu wants to exit
            return false;
        }
        return true;
    }

    MochaUtilsStatus status;
    switch (state) {
    case STATE_INIT:
        state = STATE_INIT_MOCHA;
        break;
    case STATE_INIT_MOCHA:
        status = Mocha_InitLibrary();
        if (status == MOCHA_RESULT_SUCCESS) {
            state = STATE_INIT_OTP;
            break;
        }

        stateFailure = true;
        break;
    case STATE_INIT_OTP:
        if (OTP::Init()) {
            state = STATE_INIT_SEEPROM;
            break;
        }

        stateFailure = true;
        break;
    case STATE_INIT_SEEPROM:
        if (SEEPROM::Init()) {
            state = STATE_INIT_MEMORY_DEVICE;
            break;
        }

        stateFailure = true;
        break;
    case STATE_INIT_MEMORY_DEVICE:
        if (MemoryDevice::Init()) {
            state = STATE_LOAD_MENU;
            break;
        }

        stateFailure = true;
        break;
    case STATE_LOAD_MENU:
        menuScreen = std::make_unique<MenuScreen>();
        break;
    case STATE_IN_MENU:
        break;
    };

    return true;
}

void MainScreen::DrawStatus(std::string status, SDL_Color color)
{
    Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, color, status, Gfx::ALIGN_CENTER);
}
