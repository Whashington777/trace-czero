#pragma once

namespace layout
{
    inline constexpr float window_w    = 700.f;
    inline constexpr float window_h    = 430.f;
    inline constexpr float shell_round = 12.f;

    inline constexpr float sidebar_w           = 68.f;
    inline constexpr float sidebar_pad         = 14.f;
    inline constexpr float sidebar_tab_h       = 44.f;
    inline constexpr float sidebar_tab_w       = 44.f;
    inline constexpr float sidebar_tab_icon    = 22.f;
    inline constexpr float sidebar_tab_round   = sidebar_tab_w * 0.5f;
    inline constexpr float sidebar_logo_size   = 28.f;
    inline constexpr float sidebar_logo_gap    = 14.f;
    inline constexpr float sidebar_tabs_gap    = 8.f;

    inline constexpr float sidebar_exit_h        = 44.f;
    inline constexpr float sidebar_exit_round    = sidebar_exit_h * 0.5f;
    inline constexpr float sidebar_bottom_pad    = 14.f;

    inline constexpr float content_gap       = 0.f;
    inline constexpr float content_pad_x     = 14.f;
    inline constexpr float content_pad_y     = 12.f;
    inline constexpr float scroll_bottom_pad = 52.f;

    inline constexpr float column_gap        = 10.f;
    inline constexpr float content_margin_right = 10.f;

    inline constexpr float section_header_h  = 12.f;
    inline constexpr float section_header_gap= 8.f;
    inline constexpr float box_pad_y         = 8.f;
    inline constexpr float box_pad_x         = 14.f;
    inline constexpr float row_label_y_nudge = -3.f;
    inline constexpr float box_round         = 6.f;
    inline constexpr float settings_row_h    = 30.f;
    inline constexpr float separator_h       = 1.f;
    inline constexpr float row_separator_gap = 5.f;

    inline constexpr float control_h           = 24.f;
    inline constexpr float toggle_w            = 35.f;
    inline constexpr float toggle_h            = 20.f;
    inline constexpr float toggle_round        = toggle_h * 0.5f;
    inline constexpr float toggle_knob_r       = 7.f;
    inline constexpr float toggle_knob_travel  = 15.f;
    inline constexpr float toggle_knob_inset   = 10.f;
    inline constexpr float control_round       = 4.f;
    inline constexpr float slider_h          = 6.f;
    inline constexpr float slider_track_h    = 2.f;
    inline constexpr float slider_value_w    = 32.f;
    inline constexpr float slider_w          = 108.f;
    inline constexpr float compact_slider_w  = 92.f;
    inline constexpr float combo_h           = control_h;
    inline constexpr float combo_w           = 124.f;
    inline constexpr float compact_combo_w   = 108.f;
    inline constexpr float combo_round       = control_round;
    inline constexpr float keybind_w         = combo_w;
    inline constexpr float compact_keybind_w = compact_combo_w;
    inline constexpr float keybind_icon_size = 13.f;
    inline constexpr float keybind_icon_pad  = 8.f;
    inline constexpr float dropdown_item_h   = 28.f;
    inline constexpr float color_swatch_size = control_h;
    inline constexpr float button_h          = control_h;
    inline constexpr float config_list_h     = 228.f;
    inline constexpr float config_item_h     = 48.f;
    inline constexpr float config_item_gap   = 6.f;
    inline constexpr float config_search_h   = 32.f;
    inline constexpr float config_card_round = 5.f;
    inline constexpr float config_manage_h     = 258.f;

    inline constexpr const char* main_window_id = "##imgui_portfolio_8";
    inline constexpr const char* content_child_id = "##p8_content_scroll";

    inline float row_pitch()
    {
        return settings_row_h + row_separator_gap * 2.f + separator_h;
    }

    inline float settings_box_height(int rows)
    {
        if (rows <= 0)
            return box_pad_y * 2.f;
        return box_pad_y * 2.f
             + (float)rows * settings_row_h
             + (float)(rows - 1) * (row_separator_gap * 2.f + separator_h);
    }

    inline float content_x()
    {
        return sidebar_w + content_gap;
    }
}
