#include "visuals_page.h"

#include "helpers/settings_panel.h"
#include "helpers/widgets.h"
#include "theme/layout.h"

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

    void build_esp(ImDrawList* dl, const ImRect& box, visuals_page::state& s, bool& blocks_drag, int rows)
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

        widgets::checkbox_row("vis_esp", "Enable ESP", &s.esp_enabled, next(), blocks_drag);
        widgets::checkbox_row("vis_box", "Bounding box", &s.box_esp, next(), blocks_drag);
        widgets::checkbox_row("vis_corner", "Corner box", &s.corner_box, next(), blocks_drag);
        widgets::checkbox_row("vis_name", "Name tags", &s.name_esp, next(), blocks_drag);
        widgets::checkbox_row("vis_health", "Health bar", &s.health_bar, next(), blocks_drag);
        widgets::checkbox_row("vis_skel", "Skeleton", &s.skeleton, next(), blocks_drag);
        widgets::checkbox_row("vis_head", "Head dot", &s.head_dot, next(), blocks_drag);
        widgets::checkbox_row("vis_trace", "Tracers", &s.tracers, next(), blocks_drag);
    }

    void build_filters(ImDrawList* dl, const ImRect& box, visuals_page::state& s, bool& blocks_drag, int rows)
    {
        static const char* box_styles[] = { "2D box", "3D box", "Filled" };
        static const char* tracer_origins[] = { "Bottom", "Center", "Crosshair" };

        int i = 0;
        auto next = [&]()
        {
            const ImRect row = settings_panel::row_rect(box, i);
            if (i + 1 < rows)
                settings_panel::draw_separator(dl, row);
            ++i;
            return row;
        };

        widgets::checkbox_row("vis_team", "Team check", &s.team_check, next(), blocks_drag);
        widgets::checkbox_row("vis_visible", "Visible only", &s.visible_only, next(), blocks_drag);
        widgets::slider_row("vis_dist", widgets::basic_mode_enabled()?"Max distance (m)":"Max distance", &s.max_distance, 50.f, 2000.f, next(), blocks_drag);
        widgets::combo_row("vis_box_style", "Box style", &s.box_style, box_styles, IM_ARRAYSIZE(box_styles), next(), blocks_drag);
        widgets::combo_row("vis_trace_origin", "Tracer origin", &s.tracer_origin, tracer_origins, IM_ARRAYSIZE(tracer_origins), next(), blocks_drag);
    }

    void build_chams(ImDrawList* dl, const ImRect& box, visuals_page::state& s, bool& blocks_drag, int rows)
    {
        static const char* materials[] = { "Flat", "Glow", "Glass", "Forcefield" };

        int i = 0;
        auto next = [&]()
        {
            const ImRect row = settings_panel::row_rect(box, i);
            if (i + 1 < rows)
                settings_panel::draw_separator(dl, row);
            ++i;
            return row;
        };

        widgets::checkbox_row("vis_chams", "Enable chams", &s.chams_enabled, next(), blocks_drag);
        widgets::checkbox_row("vis_chams_vis", "Visible layer", &s.chams_visible, next(), blocks_drag);
        widgets::checkbox_row("vis_chams_occ", "Occluded layer", &s.chams_occluded, next(), blocks_drag);
        widgets::checkbox_row("vis_chams_wire", "Wireframe", &s.chams_wireframe, next(), blocks_drag);
        widgets::combo_row("vis_material", "Material", &s.chams_material, materials, IM_ARRAYSIZE(materials), next(), blocks_drag);
        widgets::slider_row("vis_chams_alpha", "Opacity", &s.chams_opacity, 0.1f, 1.f, next(), blocks_drag);
    }

    void build_hud(ImDrawList* dl, const ImRect& box, visuals_page::state& s, bool& blocks_drag, int rows)
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

        widgets::checkbox_row("vis_cross", "Custom crosshair", &s.crosshair, next(), blocks_drag);
        widgets::checkbox_row("vis_fov", "FOV circle", &s.fov_circle, next(), blocks_drag);
        widgets::checkbox_row("vis_hit", "Hitmarker", &s.hitmarker, next(), blocks_drag);
        widgets::checkbox_row("vis_mark", "Watermark", &s.watermark, next(), blocks_drag);
        widgets::checkbox_row("vis_stream", "Streamproof", &s.streamproof, next(), blocks_drag);
        widgets::slider_row("vis_fov_r", "FOV radius", &s.fov_radius, 20.f, 400.f, next(), blocks_drag);
    }
}

namespace visuals_page
{
    void draw(state& s, bool& blocks_window_drag)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 content_pos = settings_panel::content_origin();
        const float left_w  = settings_panel::column_left_width();
        const float right_w = settings_panel::column_right_width();

        draw_panel(dl, content_pos, left_w, "Player ESP", 8, s, blocks_window_drag, build_esp);

        const ImVec2 right_top = { content_pos.x + left_w + layout::column_gap, content_pos.y };
        draw_panel(dl, right_top, right_w, "Filters", 5, s, blocks_window_drag, build_filters);

        const float top_block = settings_panel::panel_block_h("Player ESP", 8) + 12.f;
        const ImVec2 left_bottom = { content_pos.x, content_pos.y + top_block };
        draw_panel(dl, left_bottom, left_w, "Chams", 6, s, blocks_window_drag, build_chams);

        const ImVec2 right_bottom = {
            right_top.x,
            right_top.y + settings_panel::panel_block_h("Filters", 5) + 12.f };
        draw_panel(dl, right_bottom, right_w, "HUD", 6, s, blocks_window_drag, build_hud);

        const float bottom = ImMax(left_bottom.y + settings_panel::panel_block_h("Chams", 6),
            right_bottom.y + settings_panel::panel_block_h("HUD", 6));
        settings_panel::finalize_scroll(bottom);
    }
}
