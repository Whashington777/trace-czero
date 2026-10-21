#pragma once

#include "imgui.h"

namespace fonts
{
    void init();
    void shutdown();

    void  sync_ui_scale(float factor);
    float design_to_pixel(float design_size);
    float size(ImFont* font);

    ImFont* inter(float design_size, bool bold = false);
    ImFont* icon(float design_size);
    ImFont* logo(float design_size);
}
