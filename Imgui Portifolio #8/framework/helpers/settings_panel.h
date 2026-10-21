#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace settings_panel
{
    void draw_separator(ImDrawList* dl, const ImRect& row);
    ImRect row_rect(const ImRect& box, int index);
    float header_block_h(const char* header);
    float panel_block_h(const char* header, int rows);
    ImRect draw_box(ImDrawList* dl, const ImVec2& pos, float width, int rows);
    void draw_header(ImDrawList* dl, const ImVec2& pos, const char* title);

    ImVec2 content_origin();
    float content_width();
    float column_width();
    float column_left_width();
    float column_right_width();
    void finalize_scroll(float content_bottom_y);
}
