#include <string_view>
#include <cstdint>
#include "widgets.h"

#include "helpers/fonts.h"
#include "helpers/glow.h"
#include "helpers/icons.h"
#include "theme/colors.h"
#include "theme/layout.h"
#include "theme/ui_scale.h"

#include "imgui_internal.h"


#include <cmath>
#include <map>
#include <string>

namespace
{
    bool basic_mode = false;
    struct FeatureScope {
        bool disabled;
        explicit FeatureScope(const char* id) : disabled(false) {
            static constexpr std::string_view supported[] = {
                "recoil_enabled", "recoil_start", "aim_recoil", "aim_enabled", "aim_smooth", "aim_key", "aim_fov", "aim_dist", "aim_team",
                "vis_esp", "vis_box", "vis_name", "vis_trace", "vis_team", "vis_visible", "vis_dist", "vis_trace_origin",
                "vis_cross", "vis_fov", "vis_mark", "set_accent", "set_scale", "set_menu_key"
            };
            if(basic_mode) {disabled=true;for(auto name:supported)if(name==id)disabled=false;}
            ImGui::BeginDisabled(disabled);
        }
        ~FeatureScope(){ImGui::EndDisabled();}
    };

    struct anim_state
    {
        float active = 0.f;
        float hover  = 0.f;
        float check  = 0.f;
        float fill   = 0.f;
        float pulse  = 0.f;
        float open   = 0.f;
    };

    std::map<ImGuiID, anim_state> g_anims;

    void smooth(float& v, float target, float speed)
    {
        (void)speed;
        v = target;
    }

    void ease(float& v, float target, float speed)
    {
        smooth(v, target, speed);
    }

    void draw_row_hover(ImDrawList* dl, const ImRect& row, float hover_t)
    {
        if (hover_t < 0.01f)
            return;

        dl->AddRectFilled(row.Min, row.Max,
            ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 0.025f * hover_t)), ui_scale::s(layout::control_round));
    }

    ImVec2 text_pos_in_row(ImFont* font, float size, const ImRect& row, const char* text, float x, float y_nudge = 0.f)
    {
        const ImVec2 ts = font->CalcTextSizeA(size, FLT_MAX, 0.f, text);
        return { x, row.Min.y + (row.GetHeight() - ts.y) * 0.5f + y_nudge };
    }

    ImVec2 label_pos_in_row(ImFont* font, float size, const ImRect& row, const char* text, float x)
    {
        return text_pos_in_row(font, size, row, text, x, ui_scale::s(layout::row_label_y_nudge));
    }

    ImVec4 label_color(bool active, bool hovered)
    {
        if (active || hovered)
            return colors::text;
        return colors::text_muted;
    }

    ImRect toggle_rect(const ImRect& row)
    {
        const float w  = ui_scale::s(layout::toggle_w);
        const float h  = ui_scale::s(layout::toggle_h);
        const float cy = row.GetCenter().y;
        const float pad = ui_scale::s(layout::box_pad_x);
        return {
            { row.Max.x - pad - w, cy - h * 0.5f },
            { row.Max.x - pad, cy + h * 0.5f } };
    }

    void draw_icon_centered(ImDrawList* dl, ImFont* font, const ImRect& rect,
        const char* glyph, ImU32 col)
    {
        if (!font || !glyph)
            return;

        const float render = fonts::size(font);
        const ImVec2 ts = font->CalcTextSizeA(render, FLT_MAX, 0.f, glyph);
        const ImVec2 pos = {
            rect.Min.x + (rect.GetWidth() - ts.x) * 0.5f,
            rect.Min.y + (rect.GetHeight() - ts.y) * 0.5f };
        dl->AddText(font, render, pos, col, glyph);
    }

    struct combo_popup
    {
        bool  open      = false;
        float alpha     = 0.f;
        float slide     = 0.f;
        float opened_at = 0.f;
    };

    std::map<std::string, combo_popup> g_combos;
    std::map<ImGuiID, bool> g_keybind_capture;
    std::map<ImGuiID, float> g_slider_drag_value;
    std::map<ImGuiID, float> g_slider_drag_mouse_x;
    std::map<ImGuiID, float> g_slider_drag_track_w;

    const char* key_name(int key)
    {
        return key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END
            ? ImGui::GetKeyName(static_cast<ImGuiKey>(key)) : "None";
    }

    void close_other_combos(const char* keep)
    {
        for (auto& [id, state] : g_combos)
        {
            if (id != keep)
                state.open = false;
        }
    }

    bool combo_dropdown(const char* id, const ImRect& anchor, int* index, const char* const items[], int count, bool& blocks_drag)
    {
        combo_popup& state = g_combos[id];
        smooth(state.alpha, state.open ? 1.f : 0.f, 18.f);
        smooth(state.slide, state.open ? 0.f : 8.f, 20.f);
        if (state.alpha < 0.01f && !state.open)
            return false;

        const float item_h = ui_scale::s(layout::dropdown_item_h);
        const float popup_h = (float)count * item_h + ui_scale::s(8.f);
        ImGui::SetNextWindowPos({ anchor.Min.x, anchor.Max.y + ui_scale::s(4.f) + state.slide }, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ anchor.GetWidth(), popup_h }, ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui_scale::s(layout::combo_round));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { ui_scale::s(4.f), ui_scale::s(4.f) });
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, state.alpha * state.alpha);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.f, 0.f, 0.f, 0.f });

        char win_id[64];
        ImFormatString(win_id, IM_ARRAYSIZE(win_id), "##combo_%s", id);

        if (!ImGui::Begin(win_id, nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav))
        {
            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);
            return false;
        }

        blocks_drag = true;

        const ImRect panel = ImGui::GetCurrentWindow()->Rect();
        ImDrawList* wdl = ImGui::GetWindowDrawList();
        wdl->AddRectFilled(panel.Min, panel.Max, ImGui::GetColorU32(colors::dropdown_bg), ui_scale::s(layout::combo_round));

        if (state.open && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            if (!panel.Contains(mouse) && !anchor.Contains(mouse))
                state.open = false;
        }

        ImFont* font = fonts::inter(13.f);
        ImFont* icon_font = fonts::icon(13.f);
        const float item_text = fonts::size(font);
        bool changed = false;

        for (int i = 0; i < count; ++i)
        {
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            const ImRect item_bb(pos, pos + ImVec2(anchor.GetWidth() - ui_scale::s(8.f), item_h));
            ImGuiID item_id = ImGui::GetID((void*)(intptr_t)(i + 1));
            ImGui::ItemSize(item_bb);
            ImGui::ItemAdd(item_bb, item_id);

            bool hovered = false, held = false;
            const bool pressed = ImGui::ButtonBehavior(item_bb, item_id, &hovered, &held);
            if (hovered)
                blocks_drag = true;

            anim_state& item_anim = g_anims[item_id];
            smooth(item_anim.hover, hovered ? 1.f : 0.f, 20.f);

            const bool selected = (*index == i);
            if (hovered || selected || item_anim.hover > 0.01f)
            {
                const ImVec4 bg = selected
                    ? ImVec4(colors::accent.x, colors::accent.y, colors::accent.z, 0.14f)
                    : ImVec4(1.f, 1.f, 1.f, 0.03f + item_anim.hover * 0.03f);
                wdl->AddRectFilled(item_bb.Min, item_bb.Max, ImGui::GetColorU32(bg), ui_scale::s(layout::combo_round));
            }

            if (font)
            {
                wdl->AddText(font, item_text,
                    text_pos_in_row(font, item_text, item_bb, items[i], item_bb.Min.x + ui_scale::s(10.f)),
                    ImGui::GetColorU32(label_color(selected, hovered)),
                    items[i]);
            }

            if (selected && icon_font)
            {
                const ImRect icon_rect(
                    { item_bb.Max.x - ui_scale::s(24.f), item_bb.Min.y },
                    { item_bb.Max.x - ui_scale::s(6.f), item_bb.Max.y });
                draw_icon_centered(wdl, icon_font, icon_rect, icons::check,
                    ImGui::GetColorU32(hovered ? colors::text : colors::text_muted));
            }

            if (pressed)
            {
                *index = i;
                state.open = false;
                changed = true;
            }

            ImGui::SetCursorScreenPos({ pos.x, pos.y + item_h });
        }

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
        return changed;
    }
}

namespace widgets
{
    void set_basic_mode(bool enabled) { basic_mode=enabled; }
    bool basic_mode_enabled() { return basic_mode; }
    bool capturing_key() {
        for (const auto& entry : g_keybind_capture) if (entry.second) return true;
        return false;
    }
    void draw_section_header(ImDrawList* dl, const ImVec2& pos, const char* title)
    {
        ImFont* font = fonts::inter(layout::section_header_h, true);
        if (!font)
            font = ImGui::GetFont();

        if (font)
            dl->AddText(font, fonts::size(font), pos, ImGui::GetColorU32(colors::header_text), title);
    }

    ImRect draw_settings_box(ImDrawList* dl, const ImVec2& pos, float width, int rows)
    {
        const float height = layout::settings_box_height(rows);
        const ImRect box(pos, pos + ImVec2(width, height));
        dl->AddRectFilled(box.Min, box.Max, ImGui::GetColorU32(colors::box), layout::box_round);
        return box;
    }

    bool checkbox_row(const char* id, const char* label, bool* value, const ImRect& row, bool& blocks_drag)
    {
        FeatureScope feature(id);
        ImGuiID gid = ImGui::GetID(id);
        ImGui::ItemAdd(row, gid);

        bool hovered = false, held = false;
        const bool pressed = ImGui::ButtonBehavior(row, gid, &hovered, &held);
        if (pressed)
            *value = !*value;

        if (hovered || held)
            blocks_drag = true;

        anim_state& anim = g_anims[gid];
        ease(anim.active, *value ? 1.f : 0.f, 16.f);
        ease(anim.hover, hovered || held ? 1.f : 0.f, 24.f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        draw_row_hover(dl, row, anim.hover * 0.65f);

        ImFont* font = fonts::inter(13.f);
        const float label_size = fonts::size(font);
        if (font)
        {
            dl->AddText(font, label_size,
                label_pos_in_row(font, label_size, row, label, row.Min.x + ui_scale::s(layout::box_pad_x)),
                ImGui::GetColorU32(label_color(*value, hovered)),
                label);
        }

        const ImRect toggle = toggle_rect(row);
        const float toggle_round = ui_scale::s(layout::toggle_round);

        const ImVec4 track_bg = *value
            ? colors::control
            : ImLerp(colors::control, colors::control_hover, anim.hover);
        dl->AddRectFilled(toggle.Min, toggle.Max, ImGui::GetColorU32(track_bg), toggle_round);

        if (anim.active > 0.01f)
        {
            dl->AddRectFilled(toggle.Min, toggle.Max,
                ImGui::GetColorU32(ImVec4(
                    colors::accent.x, colors::accent.y, colors::accent.z, 0.10f * anim.active)),
                toggle_round);
            glow::draw_accent_rect(dl, toggle, toggle_round, anim.active * 0.55f);
        }

        const ImVec4 knob_off = ImLerp(colors::text_muted, colors::circle_checkbox_hover, anim.hover);
        const ImVec4 knob_col = ImLerp(knob_off, colors::accent, anim.active);
        const float knob_x = toggle.Min.x + ui_scale::s(layout::toggle_knob_inset)
            + ui_scale::s(layout::toggle_knob_travel) * anim.active;
        dl->AddCircleFilled({ knob_x, toggle.GetCenter().y }, ui_scale::s(layout::toggle_knob_r),
            ImGui::GetColorU32(knob_col), 24);

        return pressed;
    }

    bool slider_row(const char* id, const char* label, float* value, float min, float max, const ImRect& row, bool& blocks_drag)
    {
        FeatureScope feature(id);
        const float slider_w    = ui_scale::s(layout::slider_w);
        const float right       = row.Max.x - ui_scale::s(layout::box_pad_x);
        const float track_right = right;
        const float track_left  = track_right - slider_w;
        const float value_x     = track_left - ui_scale::s(layout::slider_value_w) - ui_scale::s(8.f);

        const ImRect track_bb(
            { track_left, row.GetCenter().y - ui_scale::s(layout::slider_h) * 0.5f },
            { track_right, row.GetCenter().y + ui_scale::s(layout::slider_h) * 0.5f });

        ImGuiID gid = ImGui::GetID(id);
        ImGui::ItemAdd(row, gid);

        const bool row_hovered = ImGui::IsMouseHoveringRect(row.Min, row.Max);

        bool hovered = false, held = false;
        ImGui::ButtonBehavior(track_bb, gid, &hovered, &held);
        if (hovered || held || row_hovered)
            blocks_drag = true;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered)
        {
            g_slider_drag_value[gid]   = *value;
            g_slider_drag_mouse_x[gid] = ImGui::GetIO().MousePos.x;
            g_slider_drag_track_w[gid] = track_bb.GetWidth();
        }

        if (held)
        {
            if (g_slider_drag_value.find(gid) == g_slider_drag_value.end())
            {
                g_slider_drag_value[gid]   = *value;
                g_slider_drag_mouse_x[gid] = ImGui::GetIO().MousePos.x;
                g_slider_drag_track_w[gid] = track_bb.GetWidth();
            }

            const float dx = ImGui::GetIO().MousePos.x - g_slider_drag_mouse_x[gid];
            const float range = max - min;
            const float track_w = ImMax(g_slider_drag_track_w[gid], 1.f);
            *value = ImClamp(g_slider_drag_value[gid] + (dx / track_w) * range, min, max);
        }
        else
        {
            g_slider_drag_value.erase(gid);
            g_slider_drag_mouse_x.erase(gid);
            g_slider_drag_track_w.erase(gid);
        }

        anim_state& anim = g_anims[gid];
        ease(anim.hover, hovered || held || row_hovered ? 1.f : 0.f, 18.f);
        const float target_t = ImClamp((*value - min) / (max - min), 0.f, 1.f);
        ease(anim.fill, target_t, held ? 28.f : 16.f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        draw_row_hover(dl, row, anim.hover * 0.55f);

        ImFont* font = fonts::inter(13.f);
        const float label_size = fonts::size(font);
        const bool active = *value > min + 0.01f;
        if (font)
        {
            dl->AddText(font, label_size,
                label_pos_in_row(font, label_size, row, label, row.Min.x + ui_scale::s(layout::box_pad_x)),
                ImGui::GetColorU32(label_color(active, row_hovered || hovered || held)),
                label);
        }

        const float fill_w = track_bb.GetWidth() * anim.fill;
        const float cy = track_bb.GetCenter().y;
        const ImVec4 track_col = ImLerp(colors::control, colors::control_hover, anim.hover * 0.35f);
        const float track_h = ui_scale::s(layout::slider_track_h);
        const float slider_h = ui_scale::s(layout::slider_h);

        dl->AddRectFilled(
            { track_bb.Min.x, cy - track_h * 0.5f },
            { track_bb.Max.x, cy + track_h * 0.5f },
            ImGui::GetColorU32(track_col), track_h * 0.5f);

        if (fill_w > 0.5f)
        {
            const ImRect fill_bb(
                { track_bb.Min.x, cy - slider_h * 0.5f },
                { track_bb.Min.x + fill_w, cy + slider_h * 0.5f });

            glow::draw_accent_rect(dl, fill_bb, slider_h * 0.5f, 0.45f + anim.fill * 0.40f);

            dl->AddRectFilled(fill_bb.Min, fill_bb.Max,
                ImGui::GetColorU32(colors::accent), slider_h * 0.5f);
        }

        char buf[16];
        ImFormatString(buf, IM_ARRAYSIZE(buf), "%.0f", *value);
        if (font)
        {
            dl->AddText(font, label_size,
                text_pos_in_row(font, label_size, row, buf, value_x),
                ImGui::GetColorU32(ImLerp(colors::text_muted, colors::text, anim.hover * 0.5f + (held ? 0.5f : 0.f))),
                buf);
        }

        return held;
    }

    bool slider_int_row(const char* id, const char* label, int* value, int min, int max, const ImRect& row, bool& blocks_drag)
    {
        float temporary = static_cast<float>(*value);
        slider_row(id, label, &temporary, static_cast<float>(min), static_cast<float>(max), row, blocks_drag);
        int rounded = ImClamp(static_cast<int>(temporary + .5f), min, max);
        bool changed = rounded != *value;
        *value = rounded;
        return changed;
    }

    bool combo_row(const char* id, const char* label, int* index, const char* const items[], int count, const ImRect& row, bool& blocks_drag)
    {
        FeatureScope feature(id);
        const float combo_w = ui_scale::s(layout::combo_w);
        const float combo_h = ui_scale::s(layout::combo_h);
        const float right = row.Max.x - ui_scale::s(layout::box_pad_x);
        const ImRect btn(
            { right - combo_w, row.GetCenter().y - combo_h * 0.5f },
            { right, row.GetCenter().y + combo_h * 0.5f });

        ImGuiID gid = ImGui::GetID(id);
        ImGui::ItemAdd(row, gid);

        const bool row_hovered = ImGui::IsMouseHoveringRect(row.Min, row.Max);

        bool hovered = false, held = false;
        const bool pressed = ImGui::ButtonBehavior(btn, gid, &hovered, &held);
        if (hovered || held || row_hovered)
            blocks_drag = true;

        combo_popup& popup = g_combos[id];
        if (pressed)
        {
            close_other_combos(id);
            popup.open = !popup.open;
            popup.opened_at = (float)ImGui::GetTime();
        }

        anim_state& anim = g_anims[gid];
        ease(anim.hover, hovered || popup.open || row_hovered ? 1.f : 0.f, 18.f);
        ease(anim.open, popup.open ? 1.f : 0.f, 20.f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        draw_row_hover(dl, row, anim.hover * 0.5f);

        ImFont* font = fonts::inter(13.f);
        const float label_size = fonts::size(font);
        if (font)
        {
            dl->AddText(font, label_size,
                label_pos_in_row(font, label_size, row, label, row.Min.x + ui_scale::s(layout::box_pad_x)),
                ImGui::GetColorU32(label_color(false, row_hovered || hovered || popup.open)),
                label);
        }

        const ImVec4 btn_col = ImLerp(colors::control, colors::control_hover, anim.hover);
        dl->AddRectFilled(btn.Min, btn.Max, ImGui::GetColorU32(btn_col), ui_scale::s(layout::combo_round));

        if (anim.open > 0.02f)
        {
            dl->AddRect(btn.Min, btn.Max,
                ImGui::GetColorU32(ImVec4(colors::accent.x, colors::accent.y, colors::accent.z, 0.35f * anim.open)),
                ui_scale::s(layout::combo_round), 1.f);
        }

        const char* current = (*index >= 0 && *index < count) ? items[*index] : "";
        if (font && current)
        {
            dl->AddText(font, label_size,
                text_pos_in_row(font, label_size, btn, current, btn.Min.x + ui_scale::s(10.f)),
                ImGui::GetColorU32(label_color(true, hovered || popup.open)),
                current);
        }

        if (ImFont* icon_font = fonts::icon(12.f))
        {
            static const char* arrow = "\xEE\xAA\x8A";
            const float icon_size = fonts::size(icon_font);
            const ImVec2 ts = icon_font->CalcTextSizeA(icon_size, FLT_MAX, 0.f, arrow);
            const float rot_offset = anim.open * ui_scale::s(3.f);
            dl->AddText(icon_font, icon_size,
                { btn.Max.x - ts.x - ui_scale::s(8.f) + rot_offset * 0.2f, btn.GetCenter().y - ts.y * 0.5f },
                ImGui::GetColorU32(ImLerp(colors::text_muted, colors::accent, anim.open * 0.6f)), arrow);
        }

        return combo_dropdown(id, btn, index, items, count, blocks_drag);
    }

    bool keybind_row(const char* id, const char* label, int* key, const ImRect& row, bool& blocks_drag)
    {
        FeatureScope feature(id);
        const float keybind_w = ui_scale::s(layout::keybind_w);
        const float control_h = ui_scale::s(layout::control_h);
        const float right = row.Max.x - ui_scale::s(layout::box_pad_x);
        const ImRect btn(
            { right - keybind_w, row.GetCenter().y - control_h * 0.5f },
            { right, row.GetCenter().y + control_h * 0.5f });

        ImGuiID gid = ImGui::GetID(id);
        ImGui::ItemAdd(row, gid);

        const bool row_hovered = ImGui::IsMouseHoveringRect(row.Min, row.Max);

        bool hovered = false, held = false;
        const bool pressed = ImGui::ButtonBehavior(btn, gid, &hovered, &held);
        if (hovered || held || row_hovered)
            blocks_drag = true;

        static std::map<ImGuiID, float> capture_start;
        bool& capturing = g_keybind_capture[gid];
        if (pressed)
        {
            capturing = true;
            capture_start[gid] = (float)ImGui::GetTime();
            close_other_combos("none");
        }

        if (capturing)
        {
            for (int vk = ImGuiKey_NamedKey_BEGIN; vk < ImGuiKey_NamedKey_END; ++vk)
            {
                if (vk == ImGuiKey_MouseLeft || vk == ImGuiKey_MouseRight || vk == ImGuiKey_MouseMiddle)
                    continue;
                if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(vk), false))
                {
                    *key = vk;
                    capturing = false;
                    break;
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            {
                *key = 0;
                capturing = false;
            }
        }

        anim_state& anim = g_anims[gid];
        ease(anim.hover, hovered || capturing || row_hovered ? 1.f : 0.f, 18.f);
        anim.pulse = capturing ? anim.pulse + ImGui::GetIO().DeltaTime * 6.f : 0.f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        draw_row_hover(dl, row, anim.hover * 0.5f);

        ImFont* font = fonts::inter(13.f);
        const float label_size = fonts::size(font);
        if (font)
        {
            dl->AddText(font, label_size,
                label_pos_in_row(font, label_size, row, label, row.Min.x + ui_scale::s(layout::box_pad_x)),
                ImGui::GetColorU32(label_color(*key > 0, row_hovered || hovered || capturing)),
                label);
        }

        const ImVec4 btn_col = capturing
            ? ImVec4(colors::accent.x, colors::accent.y, colors::accent.z,
                0.22f + 0.08f * (0.5f + 0.5f * sinf(anim.pulse)))
            : ImLerp(colors::control, colors::control_hover, anim.hover);
        dl->AddRectFilled(btn.Min, btn.Max, ImGui::GetColorU32(btn_col), ui_scale::s(layout::control_round));

        if (capturing)
        {
            const float ring_a = 0.25f + 0.15f * (0.5f + 0.5f * sinf(anim.pulse * 1.4f));
            dl->AddRect(btn.Min, btn.Max,
                ImGui::GetColorU32(ImVec4(colors::accent.x, colors::accent.y, colors::accent.z, ring_a)),
                ui_scale::s(layout::control_round), 0, 1.2f);
        }

        if (ImFont* icon_font = fonts::icon(layout::keybind_icon_size))
        {
            const ImRect icon_rect(
                { btn.Min.x + ui_scale::s(layout::keybind_icon_pad), btn.Min.y },
                { btn.Min.x + ui_scale::s(layout::keybind_icon_pad) + control_h, btn.Max.y });
            draw_icon_centered(dl, icon_font, icon_rect, icons::keyboard,
                ImGui::GetColorU32(hovered || capturing ? colors::text : colors::text_muted));
        }

        const char* key_text = capturing ? "Press key..." : key_name(*key);
        if (font)
        {
            const float text_x = btn.Min.x + ui_scale::s(layout::keybind_icon_pad) + control_h + ui_scale::s(4.f);
            const ImVec4 text_col = capturing
                ? ImLerp(colors::text_muted, colors::accent, 0.5f + 0.5f * sinf(anim.pulse))
                : ImLerp(colors::text_muted, colors::text, anim.hover);
            dl->AddText(font, label_size,
                text_pos_in_row(font, label_size, btn, key_text, text_x),
                ImGui::GetColorU32(text_col),
                key_text);
        }

        return pressed;
    }

    bool color_row(const char* id, const char* label, ImVec4* value, const ImRect& row, bool& blocks_drag)
    {
        FeatureScope feature(id);
        const float swatch_size = ui_scale::s(layout::color_swatch_size);
        const float right = row.Max.x - ui_scale::s(layout::box_pad_x);
        const ImRect swatch(
            { right - swatch_size, row.GetCenter().y - swatch_size * 0.5f },
            { right, row.GetCenter().y + swatch_size * 0.5f });

        ImGuiID gid = ImGui::GetID(id);
        ImGui::ItemAdd(row, gid);

        const bool row_hovered = ImGui::IsMouseHoveringRect(row.Min, row.Max);

        bool hovered = false, held = false;
        const bool pressed = ImGui::ButtonBehavior(swatch, gid, &hovered, &held);
        if (hovered || held || row_hovered)
            blocks_drag = true;

        anim_state& anim = g_anims[gid];
        ease(anim.hover, hovered || row_hovered ? 1.f : 0.f, 18.f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        draw_row_hover(dl, row, anim.hover * 0.45f);

        ImFont* font = fonts::inter(13.f);
        const float label_size = fonts::size(font);
        if (font)
        {
            dl->AddText(font, label_size,
                label_pos_in_row(font, label_size, row, label, row.Min.x + ui_scale::s(layout::box_pad_x)),
                ImGui::GetColorU32(label_color(true, row_hovered || hovered)),
                label);
        }

        const ImVec4 frame_col = ImLerp(colors::control, colors::control_hover, anim.hover);
        const float control_round = ui_scale::s(layout::control_round);
        dl->AddRectFilled(swatch.Min, swatch.Max, ImGui::GetColorU32(frame_col), control_round);
        dl->AddRectFilled(
            { swatch.Min.x + ui_scale::s(3.f), swatch.Min.y + ui_scale::s(3.f) },
            { swatch.Max.x - ui_scale::s(3.f), swatch.Max.y - ui_scale::s(3.f) },
            ImGui::GetColorU32(*value), ImMax(0.f, control_round - 1.f));

        char popup_id[64];
        ImFormatString(popup_id, IM_ARRAYSIZE(popup_id), "##color_%s", id);
        if (pressed)
            ImGui::OpenPopup(popup_id);

        ImGui::SetNextWindowPos({ swatch.Max.x - 180.f, swatch.Max.y + 6.f }, ImGuiCond_Appearing);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, layout::combo_round);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.f, 10.f });
        ImGui::PushStyleColor(ImGuiCol_PopupBg, colors::dropdown_bg);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colors::control);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, colors::control_hover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, colors::control_hover);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, colors::accent);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, colors::accent);

        if (ImGui::BeginPopup(popup_id))
        {
            blocks_drag = true;
            ImGui::ColorPicker4("##picker", (float*)value,
                ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB);
            ImGui::EndPopup();
        }

        ImGui::PopStyleColor(6);
        ImGui::PopStyleVar(2);
        return pressed;
    }

    bool button_row(const char* id, const char* label, const ImRect& row, bool& blocks_drag, ImVec4 color)
    {
        FeatureScope feature(id);
        const float btn_w = ui_scale::s(96.f);
        const float btn_h = ui_scale::s(layout::button_h);
        const float right = row.Max.x - ui_scale::s(layout::box_pad_x);
        const ImRect btn(
            { right - btn_w, row.GetCenter().y - btn_h * 0.5f },
            { right, row.GetCenter().y + btn_h * 0.5f });

        ImGuiID gid = ImGui::GetID(id);
        ImGui::ItemAdd(btn, gid);

        bool hovered = false, held = false;
        const bool pressed = ImGui::ButtonBehavior(btn, gid, &hovered, &held);
        if (hovered || held)
            blocks_drag = true;

        anim_state& anim = g_anims[gid];
        ease(anim.hover, hovered || held ? 1.f : 0.f, 18.f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImFont* font = fonts::inter(13.f);

        const bool custom = color.w > 0.f;
        const ImVec4 base = custom ? color : colors::control;
        const ImVec4 hover_col = custom
            ? ImVec4(color.x * 1.08f, color.y * 1.08f, color.z * 1.08f, color.w)
            : colors::control_hover;
        const ImVec4 btn_col = ImLerp(base, hover_col, anim.hover);
        dl->AddRectFilled(btn.Min, btn.Max, ImGui::GetColorU32(btn_col), ui_scale::s(layout::control_round));

        if (font)
        {
            const float label_size = fonts::size(font);
            const ImVec2 ts = font->CalcTextSizeA(label_size, FLT_MAX, 0.f, label);
            dl->AddText(font, label_size,
                { btn.Min.x + (btn.GetWidth() - ts.x) * 0.5f, btn.GetCenter().y - ts.y * 0.5f },
                ImGui::GetColorU32(custom ? colors::text : label_color(true, hovered)),
                label);
        }

        return pressed;
    }

    bool text_input_row(const char* id, const char* label, char* buf, int buf_size, const ImRect& row, bool& blocks_drag)
    {
        FeatureScope feature(id);
        const float field_w = ui_scale::s(168.f);
        const float control_h = ui_scale::s(layout::control_h);
        const float right   = row.Max.x - ui_scale::s(layout::box_pad_x);
        const ImRect field(
            { right - field_w, row.GetCenter().y - control_h * 0.5f },
            { right, row.GetCenter().y + control_h * 0.5f });

        ImGuiID gid = ImGui::GetID(id);

        const bool row_hovered = ImGui::IsMouseHoveringRect(row.Min, row.Max);
        static std::map<ImGuiID, float> input_hover;
        float& input_h = input_hover[gid];
        const bool field_hovered = ImGui::IsMouseHoveringRect(field.Min, field.Max);
        input_h = ImLerp(input_h, row_hovered || field_hovered ? 1.f : 0.f, ImMin(1.f, ImGui::GetIO().DeltaTime * 18.f));

        if (row_hovered || field_hovered)
            blocks_drag = true;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        draw_row_hover(dl, row, input_h * 0.45f);

        ImFont* font = fonts::inter(13.f);
        const float label_size = fonts::size(font);
        if (font)
        {
            dl->AddText(font, label_size,
                label_pos_in_row(font, label_size, row, label, row.Min.x + ui_scale::s(layout::box_pad_x)),
                ImGui::GetColorU32(label_color(false, row_hovered)),
                label);
        }

        const ImVec4 field_col = ImLerp(colors::control, colors::control_hover, input_h);
        dl->AddRectFilled(field.Min, field.Max, ImGui::GetColorU32(field_col), ui_scale::s(layout::control_round));

        ImGui::SetCursorScreenPos({ field.Min.x + ui_scale::s(8.f), field.Min.y + ui_scale::s(4.f) });
        ImGui::PushItemWidth(field.GetWidth() - ui_scale::s(16.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Text, colors::text);

        char input_id[64];
        ImFormatString(input_id, IM_ARRAYSIZE(input_id), "##input_%s", id);
        if (font)
            ImGui::PushFont(font, font->LegacySize);
        const bool changed = ImGui::InputText(input_id, buf, (size_t)buf_size);
        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
            blocks_drag = true;
        if (font)
            ImGui::PopFont();

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(2);
        ImGui::PopItemWidth();
        return changed;
    }
}
