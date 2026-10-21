#pragma once

namespace aim_page
{
    struct state
    {
        bool  enabled        = false;
        float smoothing      = 50.f;
        int   aimbone        = 0;
        int   keybind        = 0;

        bool  visible_only   = true;
        bool  silent_aim     = false;
        bool  auto_shoot     = false;
        bool  ignore_team    = false;
        bool  prediction     = true;
        bool  ignore_knocked = true;
        bool  auto_scope     = false;
        float fov            = 35.f;
        float hitchance      = 65.f;
        float max_distance   = 250.f;
        bool  recoil_enabled = false;
        int   recoil_start_bullet = 1;
        float recoil_control = 100.f;
        int   target_priority= 0;
        int   force_body     = 0;
    };

    void draw(state& s, bool& blocks_window_drag);
}
