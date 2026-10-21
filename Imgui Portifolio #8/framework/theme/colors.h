#pragma once

#include "imgui.h"

namespace colors
{
    inline ImVec4 accent = { 169.f / 255.f, 128.f / 255.f, 255.f / 255.f, 1.f };

    
    inline constexpr ImVec4 bg            = { 20.f / 255.f, 20.f / 255.f, 23.f / 255.f, 1.f };
    inline constexpr ImVec4 panel         = { 20.f / 255.f, 20.f / 255.f, 23.f / 255.f, 1.f };
    inline constexpr ImVec4 sidebar       = { 24.f / 255.f, 24.f / 255.f, 28.f / 255.f, 1.f };
    inline constexpr ImVec4 box           = { 24.f / 255.f, 24.f / 255.f, 28.f / 255.f, 1.f };
    inline constexpr ImVec4 control       = { 33.f / 255.f, 33.f / 255.f, 39.f / 255.f, 1.f };
    inline constexpr ImVec4 control_hover = { 44.f / 255.f, 44.f / 255.f, 52.f / 255.f, 1.f };
    inline constexpr ImVec4 circle_checkbox_hover = { 95.f / 255.f, 95.f / 255.f, 117.f / 255.f, 1.f };
    inline constexpr ImVec4 widget        = control;
    inline constexpr ImVec4 widget_hover  = control_hover;
    inline constexpr ImVec4 text          = { 0.92f, 0.92f, 0.94f, 1.f };
    inline constexpr ImVec4 text_muted    = { 99.f / 255.f, 99.f / 255.f, 118.f / 255.f, 1.f };
    inline constexpr ImVec4 header_text   = { 130.f / 255.f, 130.f / 255.f, 155.f / 255.f, 1.f };
    inline constexpr ImVec4 separator     = { 35.f / 255.f, 35.f / 255.f, 43.f / 255.f, 0.55f };
    inline constexpr ImVec4 sidebar_sep   = { 35.f / 255.f, 35.f / 255.f, 43.f / 255.f, 0.65f };
    inline constexpr ImVec4 dropdown_bg   = { 28.f / 255.f, 28.f / 255.f, 33.f / 255.f, 1.f };
    inline constexpr ImVec4 config_sel    = { 1.f, 1.f, 1.f, 0.05f };
    inline constexpr ImVec4 config_hover  = { 1.f, 1.f, 1.f, 0.03f };
    inline constexpr ImVec4 success       = { 0.35f, 0.78f, 0.48f, 1.f };
    inline constexpr ImVec4 danger        = { 0.82f, 0.28f, 0.30f, 1.f };
}
