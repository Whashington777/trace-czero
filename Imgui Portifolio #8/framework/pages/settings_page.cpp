#include "settings_page.h"

#include "helpers/settings_panel.h"
#include "helpers/widgets.h"
#include "theme/colors.h"
#include "theme/layout.h"
#include "theme/ui_scale.h"

namespace
{
    template<typename State, typename Builder>
    void draw_panel(ImDrawList* dl, const ImVec2& pos, float width, const char* header,
        int rows, State& s, bool& blocks_drag, Builder build)
    {
        settings_panel::draw_header(dl, pos, header);
        const ImVec2 box_pos = { pos.x, pos.y + settings_panel::header_block_h(header) };
        const ImRect box = settings_panel::draw_box(dl, box_pos, width, rows);
        build(dl, box, s, blocks_drag, rows);
    }

    void build_appearance(ImDrawList* dl, const ImRect& box, settings_page::state& s, bool& blocks_drag, int rows)
    {
        static const char* themes[] = { "Lucid", "Midnight", "Crimson", "Arctic" };
        static const char* languages[] = { "English", "Russian" };

        int i = 0;
        auto next = [&]()
        {
            const ImRect row = settings_panel::row_rect(box, i);
            if (i + 1 < rows)
                settings_panel::draw_separator(dl, row);
            ++i;
            return row;
        };

        widgets::color_row("set_accent", "Accent color", &s.accent_color, next(), blocks_drag);
        colors::accent = s.accent_color;
        widgets::slider_row("set_scale", "UI scale", &s.ui_scale, 80.f, 200.f, next(), blocks_drag);
        widgets::slider_row("set_opacity", "Menu opacity", &s.menu_opacity, 40.f, 100.f, next(), blocks_drag);
        widgets::slider_row("set_anim", "Animation speed", &s.animation_speed, 0.f, 200.f, next(), blocks_drag);
        widgets::slider_row("set_round", "Corner rounding", &s.corner_rounding, 0.f, 16.f, next(), blocks_drag);
        widgets::combo_row("set_theme", "Theme preset", &s.theme_preset, themes, IM_ARRAYSIZE(themes), next(), blocks_drag);
        widgets::combo_row("set_lang", "Language", &s.language, languages, IM_ARRAYSIZE(languages), next(), blocks_drag);
    }

    void build_misc(ImDrawList* dl, const ImRect& box, settings_page::state& s, bool& blocks_drag, int rows)
    {
        int i = 0;
        auto next = [&]()
        {
            const ImRect row = settings_panel::row_rect(box, i);
            if (i + 1 < rows)
                settings_panel::draw_separator(dl, row);
            ++i;
            return row;
        };

        widgets::keybind_row("set_menu_key", "Menu key", &s.menu_key, next(), blocks_drag);
        widgets::checkbox_row("set_watermark", "Show watermark", &s.show_watermark, next(), blocks_drag);
        widgets::checkbox_row("set_keybinds", "Show keybind list", &s.show_keybinds, next(), blocks_drag);
        widgets::checkbox_row("set_stream", "Streamproof", &s.streamproof, next(), blocks_drag);
        widgets::checkbox_row("set_discord", "Discord RPC", &s.discord_rpc, next(), blocks_drag);
        widgets::checkbox_row("set_autosave", "Auto save", &s.auto_save, next(), blocks_drag);
        widgets::checkbox_row("set_dpi", "DPI aware", &s.dpi_aware, next(), blocks_drag);
    }
}

namespace settings_page
{
    void draw(state& s, bool& blocks_window_drag)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 content_pos = settings_panel::content_origin();
        const float left_w  = settings_panel::column_left_width();
        const float right_w = settings_panel::column_right_width();

        draw_panel(dl, content_pos, left_w, "Appearance", 7, s, blocks_window_drag, build_appearance);

        const ImVec2 right_pos = { content_pos.x + left_w + ui_scale::s(layout::column_gap), content_pos.y };
        draw_panel(dl, right_pos, right_w, "Miscellaneous", 7, s, blocks_window_drag, build_misc);

        colors::accent = s.accent_color;

        const float bottom = content_pos.y + ImMax(
            settings_panel::panel_block_h("Appearance", 7),
            settings_panel::panel_block_h("Miscellaneous", 7));
        settings_panel::finalize_scroll(bottom);
    }
}
