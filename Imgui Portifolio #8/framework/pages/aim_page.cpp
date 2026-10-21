#include "aim_page.h"

#include "helpers/settings_panel.h"
#include "helpers/widgets.h"
#include "theme/layout.h"

namespace
{
    void build_aimbot_panel(ImDrawList* dl, const ImRect& box, aim_page::state& s, bool& blocks_drag, int)
    {
        static const char* bones[] = { "Head", "Neck", "Chest", "Stomach", "Pelvis" };
        const char* base_bones[] = { "Head hitbox" };

        widgets::checkbox_row("aim_enabled", "Enable aimbot", &s.enabled, settings_panel::row_rect(box, 0), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 0));

        widgets::slider_row("aim_smooth", "Smoothing", &s.smoothing, 0.f, 100.f, settings_panel::row_rect(box, 1), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 1));

        widgets::combo_row("aim_bone", "Aimbone", &s.aimbone, widgets::basic_mode_enabled()?base_bones:bones, widgets::basic_mode_enabled()?1:IM_ARRAYSIZE(bones), settings_panel::row_rect(box, 2), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 2));

        widgets::keybind_row("aim_key", "Keybind", &s.keybind, settings_panel::row_rect(box, 3), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 3));

        widgets::slider_row("aim_fov", "FOV", &s.fov, 1.f, widgets::basic_mode_enabled()?85.f:180.f, settings_panel::row_rect(box, 4), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 4));

        widgets::slider_row("aim_hc", "Hitchance", &s.hitchance, 0.f, 100.f, settings_panel::row_rect(box, 5), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 5));

        widgets::slider_row("aim_dist", widgets::basic_mode_enabled()?"Max distance (m)":"Max distance", &s.max_distance, 0.f, 500.f, settings_panel::row_rect(box, 6), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 6));


    }

    void build_recoil_panel(ImDrawList* dl, const ImRect& box, aim_page::state& s, bool& blocks_drag, int)
    {
        widgets::checkbox_row("recoil_enabled", "Enable recoil control", &s.recoil_enabled, settings_panel::row_rect(box, 0), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 0));
        ImGui::BeginDisabled(!s.recoil_enabled);
        widgets::slider_int_row("recoil_start", "Start bullet", &s.recoil_start_bullet, 1, 25, settings_panel::row_rect(box, 1), blocks_drag);
        settings_panel::draw_separator(dl, settings_panel::row_rect(box, 1));
        widgets::slider_row("aim_recoil", "Strength (%)", &s.recoil_control, 0.f, 100.f, settings_panel::row_rect(box, 2), blocks_drag);
        ImGui::EndDisabled();
    }

    void build_targeting_panel(ImDrawList* dl, const ImRect& box, aim_page::state& s, bool& blocks_drag, int rows)
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

        widgets::checkbox_row("aim_visible", "Visible only", &s.visible_only, next(), blocks_drag);
        widgets::checkbox_row("aim_team", "Ignore team", &s.ignore_team, next(), blocks_drag);
        widgets::checkbox_row("aim_knocked", "Ignore knocked", &s.ignore_knocked, next(), blocks_drag);
        widgets::checkbox_row("aim_scope", "Auto scope", &s.auto_scope, next(), blocks_drag);
    }

    void build_advanced_panel(ImDrawList* dl, const ImRect& box, aim_page::state& s, bool& blocks_drag, int rows)
    {
        static const char* priorities[] = { "Distance", "Health", "Crosshair", "Threat" };
        static const char* force_body_items[] = { "None", "On miss", "Always", "Lethal only" };

        int i = 0;
        auto next = [&]()
        {
            const ImRect row = settings_panel::row_rect(box, i);
            if (i + 1 < rows)
                settings_panel::draw_separator(dl, row);
            ++i;
            return row;
        };

        widgets::checkbox_row("adv_silent", "Silent aim", &s.silent_aim, next(), blocks_drag);
        widgets::checkbox_row("adv_auto", "Auto shoot", &s.auto_shoot, next(), blocks_drag);
        widgets::checkbox_row("adv_pred", "Prediction", &s.prediction, next(), blocks_drag);
        widgets::combo_row("adv_priority", "Target priority", &s.target_priority, priorities, IM_ARRAYSIZE(priorities), next(), blocks_drag);
        widgets::combo_row("adv_force", "Force body", &s.force_body, force_body_items, IM_ARRAYSIZE(force_body_items), next(), blocks_drag);
    }

    template<typename State, typename Builder>
    void draw_panel(ImDrawList* dl, const ImVec2& pos, float width, const char* header,
        int rows, State& s, bool& blocks_drag, Builder build)
    {
        settings_panel::draw_header(dl, pos, header);
        const ImVec2 box_pos = { pos.x, pos.y + settings_panel::header_block_h(header) };
        const ImRect box = settings_panel::draw_box(dl, box_pos, width, rows);
        build(dl, box, s, blocks_drag, rows);
    }
}

namespace aim_page
{
    void draw(state& s, bool& blocks_window_drag)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 content_pos = settings_panel::content_origin();
        const float left_w  = settings_panel::column_left_width();
        const float right_w = settings_panel::column_right_width();

        draw_panel(dl, content_pos, left_w, "Aimbot", 7, s, blocks_window_drag, build_aimbot_panel);

        const ImVec2 recoil_pos = { content_pos.x, content_pos.y + settings_panel::panel_block_h("Aimbot", 7) + 12.f };
        draw_panel(dl, recoil_pos, left_w, "Recoil", 3, s, blocks_window_drag, build_recoil_panel);

        const ImVec2 right_pos = { content_pos.x + left_w + layout::column_gap, content_pos.y };
        draw_panel(dl, right_pos, right_w, "Advanced", 5, s, blocks_window_drag, build_advanced_panel);

        const ImVec2 targeting_pos = {
            right_pos.x,
            content_pos.y + settings_panel::panel_block_h("Advanced", 5) + 12.f };
        draw_panel(dl, targeting_pos, right_w, "Targeting", 4, s, blocks_window_drag, build_targeting_panel);

        const float left_bottom = recoil_pos.y + settings_panel::panel_block_h("Recoil", 3);
        const float right_bottom = targeting_pos.y + settings_panel::panel_block_h("Targeting", 4);
        settings_panel::finalize_scroll(ImMax(left_bottom, right_bottom));
    }
}
