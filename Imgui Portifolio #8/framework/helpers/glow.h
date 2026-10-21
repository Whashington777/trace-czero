#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace glow
{
    void draw_accent_rect(ImDrawList* dl, const ImRect& rect, float round, float strength = 1.f);
    void clear_frame();
}
