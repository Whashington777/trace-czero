#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace widgets
{
    bool capturing_key();
    bool basic_mode_enabled();
    void set_basic_mode(bool enabled);
    void draw_section_header(ImDrawList* dl, const ImVec2& pos, const char* title);

    ImRect draw_settings_box(ImDrawList* dl, const ImVec2& pos, float width, int rows);

    bool checkbox_row(const char* id, const char* label, bool* value, const ImRect& row, bool& blocks_drag);
    bool slider_row(const char* id, const char* label, float* value, float min, float max, const ImRect& row, bool& blocks_drag);
    bool slider_int_row(const char* id, const char* label, int* value, int min, int max, const ImRect& row, bool& blocks_drag);
    bool combo_row(const char* id, const char* label, int* index, const char* const items[], int count, const ImRect& row, bool& blocks_drag);
    bool keybind_row(const char* id, const char* label, int* key, const ImRect& row, bool& blocks_drag);
    bool color_row(const char* id, const char* label, ImVec4* value, const ImRect& row, bool& blocks_drag);
    bool button_row(const char* id, const char* label, const ImRect& row, bool& blocks_drag, ImVec4 color = ImVec4(0.f, 0.f, 0.f, 0.f));
    bool text_input_row(const char* id, const char* label, char* buf, int buf_size, const ImRect& row, bool& blocks_drag);
}
