#pragma once

namespace visuals_page
{
    struct state
    {
        bool  esp_enabled    = true;
        bool  box_esp        = true;
        bool  corner_box     = false;
        bool  name_esp       = true;
        bool  health_bar     = true;
        bool  skeleton       = false;
        bool  head_dot       = false;
        bool  tracers        = false;
        bool  team_check     = true;
        bool  visible_only   = false;
        float max_distance   = 500.f;
        int   box_style      = 0;
        int   tracer_origin  = 0;

        bool  chams_enabled  = false;
        bool  chams_visible  = true;
        bool  chams_occluded = false;
        bool  chams_wireframe= false;
        int   chams_material = 0;
        float chams_opacity  = 0.75f;

        bool  crosshair      = true;
        bool  fov_circle     = true;
        bool  hitmarker      = true;
        bool  watermark      = true;
        bool  streamproof    = false;
        float fov_radius     = 120.f;
    };

    void draw(state& s, bool& blocks_window_drag);
}
