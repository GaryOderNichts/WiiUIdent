#pragma once

#include <SDL.h>
#include <string>

namespace Gfx
{

constexpr uint32_t SCREEN_WIDTH            = 1920;
constexpr uint32_t SCREEN_HEIGHT           = 1080;

constexpr SDL_Color COLOR_BLACK            = { 0x00, 0x00, 0x00, 0xff };
constexpr SDL_Color COLOR_WHITE            = { 0xff, 0xff, 0xff, 0xff };
constexpr SDL_Color COLOR_BACKGROUND       = { 0x1b, 0x1c, 0x20, 0xff };
constexpr SDL_Color COLOR_ALT_BACKGROUND   = { 0x33, 0x34, 0x39, 0xff };
constexpr SDL_Color COLOR_HIGHLIGHTED      = { 0x00, 0x91, 0xea, 0xff };
constexpr SDL_Color COLOR_TEXT             = { 0xf8, 0xf8, 0xf8, 0xff };
constexpr SDL_Color COLOR_ALT_TEXT         = { 0xb0, 0xb0, 0xb0, 0xff };
constexpr SDL_Color COLOR_ACCENT           = { 0x32, 0xe6, 0xa6, 0xff };
constexpr SDL_Color COLOR_ALT_ACCENT       = { 0x22, 0xb3, 0x7d, 0xff };
constexpr SDL_Color COLOR_BARS             = { 0x2f, 0x3f, 0x38, 0xff };
constexpr SDL_Color COLOR_ERROR            = { 0xff, 0x33, 0x33, 0xff };
constexpr SDL_Color COLOR_WIIU             = { 0x00, 0x95, 0xc7, 0xff };

enum AlignFlags {
    ALIGN_LEFT            =   1 << 0,
    ALIGN_RIGHT           =   1 << 1,
    ALIGN_HORIZONTAL      =   1 << 2,
    ALIGN_TOP             =   1 << 3,
    ALIGN_BOTTOM          =   1 << 4,
    ALIGN_VERTICAL        =   1 << 5,
    ALIGN_CENTER          =   ALIGN_HORIZONTAL | ALIGN_VERTICAL,
};

static constexpr inline AlignFlags operator|(AlignFlags lhs, AlignFlags rhs) {
    return static_cast<AlignFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

bool Init();

void Shutdown();

void Clear(SDL_Color color);

void Render();

void DrawRectFilled(int x, int y, int w, int h, SDL_Color color);

void DrawRect(int x, int y, int w, int h, int borderSize, SDL_Color color);

void DrawIcon(int x, int y, int size, SDL_Color color, Uint16 icon, AlignFlags align = ALIGN_CENTER, double angle = 0.0);

int GetIconWidth(int size, Uint16 icon);

static inline int GetIconHeight(int size, Uint16 icon) { return size; }

void Print(int x, int y, int size, SDL_Color color, std::string text, AlignFlags align = ALIGN_LEFT | ALIGN_TOP, bool monospace = false);

int GetTextWidth(int size, std::string text, bool monospace = false);

int GetTextHeight(int size, std::string text, bool monospace = false);

}
