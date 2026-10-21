#include "settings_panel.h"

#include "helpers/fonts.h"
#include "theme/colors.h"
#include "theme/layout.h"
#include "theme/ui_scale.h"

namespace settings_panel
{
    void draw_separator(ImDrawList* dl, const ImRect& row)
    {
        const float y = row.Max.y + ui_scale::s(layout::row_separator_gap);
        dl->AddLine(
            { row.Min.x + ui_scale::s(layout::box_pad_x), y },
            { row.Max.x - ui_scale::s(layout::box_pad_x), y },
            ImGui::GetColorU32(colors::separator), ui_scale::s(layout::separator_h));
    }

    ImRect row_rect(const ImRect& box, int index)
    {
        const float pitch = ui_scale::s(layout::row_pitch());
        const float row_h = ui_scale::s(layout::settings_row_h);
        const float y = box.Min.y + ui_scale::s(layout::box_pad_y) + (float)index * pitch;
        return {
            { box.Min.x, y },
            { box.Max.x, y + row_h } };
    }

    float header_block_h(const char* header)
    {
        ImFont* header_font = fonts::inter(layout::section_header_h, true);
        if (!header_font)
            header_font = fonts::inter(layout::section_header_h);
        const float render = fonts::size(header_font);
        const ImVec2 header_ts = header_font
            ? header_font->CalcTextSizeA(render, FLT_MAX, 0.f, header)
            : ImGui::CalcTextSize(header);
        return header_ts.y + ui_scale::s(layout::section_header_gap);
    }

    float panel_block_h(const char* header, int rows)
    {
        return header_block_h(header) + ui_scale::s(layout::settings_box_height(rows));
    }

    ImRect draw_box(ImDrawList* dl, const ImVec2& pos, float width, int rows)
    {
        const float height = ui_scale::s(layout::settings_box_height(rows));
        const ImRect box(pos, pos + ImVec2(width, height));
        dl->AddRectFilled(box.Min, box.Max, ImGui::GetColorU32(colors::box),
            ui_scale::s(layout::box_round));
        return box;
    }

    void draw_header(ImDrawList* dl, const ImVec2& pos, const char* title)
    {
        ImFont* font = fonts::inter(layout::section_header_h, true);
        if (!font)
            font = fonts::inter(layout::section_header_h);
        if (!font)
            font = ImGui::GetFont();

        if (font)
            dl->AddText(font, fonts::size(font), pos, ImGui::GetColorU32(colors::header_text), title);
    }

    ImVec2 content_origin()
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        return {
            window->Pos.x + ui_scale::s(layout::content_pad_x),
            window->Pos.y + ui_scale::s(layout::content_pad_y) - window->Scroll.y };
    }

    float content_width()
    {
        return ImGui::GetWindowWidth()
             - ui_scale::s(layout::content_pad_x)
             - ui_scale::s(layout::content_margin_right);
    }

    float column_width()
    {
        return (content_width() - ui_scale::s(layout::column_gap)) * 0.5f;
    }

    float column_left_width()
    {
        return column_width();
    }

    float column_right_width()
    {
        return column_width();
    }

    void finalize_scroll(float content_bottom_y)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const float pad_y = ui_scale::s(layout::content_pad_y);
        const float scroll_pad = ui_scale::s(layout::scroll_bottom_pad);
        const float local_bottom = content_bottom_y - window->Pos.y + window->Scroll.y - pad_y;
        ImGui::SetCursorPos({ 0.f, local_bottom + pad_y });
        ImGui::Dummy({ 1.f, scroll_pad });
    }
}
