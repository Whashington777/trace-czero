#pragma once

#include "imgui.h"

namespace settings_page
{
    struct state
    {
        ImVec4 accent_color  = { 169.f / 255.f, 128.f / 255.f, 255.f / 255.f, 1.f };
        float  ui_scale      = 100.f;
        float  menu_opacity  = 100.f;
        float  animation_speed = 100.f;
        float  corner_rounding = 6.f;
        int    theme_preset  = 0;
        int    language      = 0;
        bool   show_watermark= true;
        bool   show_keybinds = true;
        bool   streamproof   = false;
        bool   discord_rpc   = false;
        bool   auto_save     = true;
        bool   dpi_aware     = true;
        int    menu_key      = ImGuiKey_Insert;
    };

    void draw(state& s, bool& blocks_window_drag);
}
