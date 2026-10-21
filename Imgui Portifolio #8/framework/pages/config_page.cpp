#include <cstdint>
#include "config_page.h"

#include "helpers/fonts.h"
#include "helpers/icons.h"
#include "helpers/settings_panel.h"
#include "helpers/widgets.h"
#include "theme/colors.h"
#include "theme/layout.h"
#include "theme/ui_scale.h"

#include "imgui_internal.h"

#include <cstring>
#include <map>

namespace
{
    struct row_anim
    {
        float hover   = 0.f;
        float select  = 0.f;
        float visible = 1.f;
    };

    struct search_anim_state
    {
        float hover  = 0.f;
        float focus  = 0.f;
        float typed  = 0.f;
        float pulse  = 0.f;
    };

    static search_anim_state g_search_anim;

    std::map<ImGuiID, row_anim> g_row_anims;

    void copy_str(char* dst, size_t dst_size, const char* src)
    {
        if (!dst || dst_size == 0)
            return;

        size_t i = 0;
        for (; src && src[i] && i + 1 < dst_size; ++i)
            dst[i] = src[i];
        dst[i] = '\0';
    }

    bool name_exists(config_page::state& s, const char* name, int skip = -1)
    {
        for (int i = 0; i < s.count; ++i)
        {
            if (i == skip)
                continue;
            if (std::strcmp(s.names[i], name) == 0)
                return true;
        }
        return false;
    }

    bool matches_search(const char* name, const char* search)
    {
        if (!search || search[0] == '\0')
            return true;

        for (int i = 0; name[i]; ++i)
        {
            int j = 0;
            for (; search[j]; ++j)
            {
                char a = name[i + j];
                char b = search[j];
                if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
                if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
                if (!a || a != b)
                    break;
            }
            if (search[j] == '\0')
                return true;
        }
        return false;
    }

    void set_status(config_page::state& s, const char* msg)
    {
        copy_str(s.status, sizeof(s.status), msg);
        s.status_alpha = 0.f;
    }

    void draw_search_field(ImDrawList* dl, const ImRect& bar, config_page::state& s, bool& blocks_drag)
    {
        const float dt = ImGui::GetIO().DeltaTime;
        const bool mouse_on_bar = ImGui::IsMouseHoveringRect(bar.Min, bar.Max, false);
        g_search_anim.typed = ImLerp(g_search_anim.typed, s.search[0] ? 1.f : 0.f, 1.f);
        g_search_anim.pulse += dt * (g_search_anim.focus > 0.5f ? 4.f : 0.f);

        const ImVec4 bg_col = ImLerp(colors::control, colors::control_hover, g_search_anim.hover * 0.65f);
        const float control_round = ui_scale::s(layout::control_round);
        dl->AddRectFilled(bar.Min, bar.Max, ImGui::GetColorU32(bg_col), control_round);

        const float accent_mix = ImMax(g_search_anim.focus, g_search_anim.typed * 0.55f);
        if (accent_mix > 0.01f)
        {
            dl->AddRect(bar.Min, bar.Max,
                ImGui::GetColorU32(ImVec4(
                    colors::accent.x, colors::accent.y, colors::accent.z, 0.18f + accent_mix * 0.45f)),
                control_round, 1.f);
        }

        const float underline_w = bar.GetWidth() * (0.25f + g_search_anim.focus * 0.75f);
        const float pulse = 0.5f + 0.5f * sinf(g_search_anim.pulse);
        const float underline_a = (0.08f + g_search_anim.focus * 0.55f) * pulse;
        dl->AddRectFilled(
            { bar.GetCenter().x - underline_w * 0.5f, bar.Max.y - ui_scale::s(2.f) },
            { bar.GetCenter().x + underline_w * 0.5f, bar.Max.y },
            ImGui::GetColorU32(ImVec4(colors::accent.x, colors::accent.y, colors::accent.z, underline_a)),
            1.f);

        if (ImFont* icon_font = fonts::icon(13.f))
        {
            const ImVec4 icon_col = ImLerp(
                colors::text_muted,
                colors::accent,
                g_search_anim.focus * 0.65f + g_search_anim.typed * 0.35f);
            const float icon_size = fonts::size(icon_font);
            const ImVec2 ts = icon_font->CalcTextSizeA(icon_size, FLT_MAX, 0.f, icons::search);
            dl->AddText(icon_font, icon_size,
                { bar.Min.x + ui_scale::s(10.f), bar.GetCenter().y - ts.y * 0.5f },
                ImGui::GetColorU32(icon_col), icons::search);
        }

        ImGui::SetCursorScreenPos({ bar.Min.x + ui_scale::s(32.f), bar.Min.y + ui_scale::s(6.f) });
        ImGui::PushItemWidth(bar.GetWidth() - ui_scale::s(42.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 2.f });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Text, colors::text);

        if (ImFont* font = fonts::inter(13.f))
            ImGui::PushFont(font);

        ImGui::InputTextWithHint("##cfg_search_input", "Search configs...", s.search, sizeof(s.search));

        const bool input_active = ImGui::IsItemActive();
        const bool input_hovered = ImGui::IsItemHovered();
        if (input_active || input_hovered || mouse_on_bar)
            blocks_drag = true;

        g_search_anim.hover = ImLerp(g_search_anim.hover,
            (mouse_on_bar || input_hovered || input_active) ? 1.f : 0.f, 1.f);
        g_search_anim.focus = ImLerp(g_search_anim.focus, input_active ? 1.f : 0.f, 1.f);

        if (ImFont* font = fonts::inter(13.f))
            ImGui::PopFont();

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(2);
        ImGui::PopItemWidth();
    }

    void draw_config_card(ImDrawList* dl, const ImRect& item, config_page::state& s, int index,
        bool& blocks_drag)
    {
        ImGuiID id = ImGui::GetID((void*)(intptr_t)(index + 100));
        ImGui::ItemAdd(item, id);

        bool hovered = false, held = false;
        const bool pressed = ImGui::ButtonBehavior(item, id, &hovered, &held);
        if (hovered || held)
            blocks_drag = true;

        row_anim& anim = g_row_anims[id];
        const float dt = ImGui::GetIO().DeltaTime;
        const bool matched = matches_search(s.names[index], s.search);
        anim.hover   = ImLerp(anim.hover,   hovered ? 1.f : 0.f, 1.f);
        anim.select  = ImLerp(anim.select,  index == s.selected ? 1.f : 0.f, 1.f);
        anim.visible = ImLerp(anim.visible, matched ? 1.f : 0.f, 1.f);

        if (anim.visible < 0.02f)
        {
            if (pressed && matched)
                s.selected = index;
            return;
        }

        const float slide = (1.f - anim.visible) * ui_scale::s(8.f);
        const ImRect draw_item(
            { item.Min.x, item.Min.y + slide },
            { item.Max.x, item.Max.y + slide });

        auto col_a = [&](ImVec4 c)
        {
            c.w *= anim.visible;
            return ImGui::GetColorU32(c);
        };

        const ImVec4 card_bg = ImLerp(
            colors::box,
            ImVec4(colors::accent.x, colors::accent.y, colors::accent.z, 0.08f),
            anim.select);
        dl->AddRectFilled(draw_item.Min, draw_item.Max, col_a(card_bg), ui_scale::s(layout::config_card_round));

        const ImVec4 border_col = ImLerp(
            ImVec4(colors::separator.x, colors::separator.y, colors::separator.z, 0.35f),
            ImVec4(colors::accent.x, colors::accent.y, colors::accent.z, 0.45f),
            anim.select + anim.hover * 0.25f);
        dl->AddRect(draw_item.Min, draw_item.Max, col_a(border_col), ui_scale::s(layout::config_card_round), 1.f);

        ImFont* font = fonts::inter(13.f);
        ImFont* meta_font = fonts::inter(11.f);
        ImFont* icon_font = fonts::icon(14.f);

        if (icon_font)
        {
            const float icon_size = fonts::size(icon_font);
            const ImVec2 ts = icon_font->CalcTextSizeA(icon_size, FLT_MAX, 0.f, icons::folder);
            dl->AddText(icon_font, icon_size,
                { draw_item.Min.x + ui_scale::s(12.f), draw_item.GetCenter().y - ts.y * 0.5f },
                col_a(ImLerp(colors::text_muted, colors::accent, anim.select * 0.85f)),
                icons::folder);
        }

        if (font)
        {
            const float title_size = fonts::size(font);
            dl->AddText(font, title_size,
                { draw_item.Min.x + ui_scale::s(34.f), draw_item.Min.y + ui_scale::s(10.f) },
                col_a(ImLerp(colors::text_muted, colors::text, anim.select * 0.85f + anim.hover * 0.15f)),
                s.names[index]);
        }

        if (meta_font)
        {
            const float meta_size = fonts::size(meta_font);
            dl->AddText(meta_font, meta_size,
                { draw_item.Min.x + ui_scale::s(34.f), draw_item.Min.y + ui_scale::s(26.f) },
                col_a(colors::text_muted), s.dates[index]);
        }

        if (index == s.selected && font)
        {
            ImFont* badge_font = fonts::inter(10.f);
            if (!badge_font)
                badge_font = font;
            const char* badge = "Active";
            const float badge_size = fonts::size(badge_font);
            const ImVec2 badge_ts = badge_font->CalcTextSizeA(badge_size, FLT_MAX, 0.f, badge);
            const float badge_pad_x = ui_scale::s(8.f);
            const float badge_pad_y = ui_scale::s(4.f);
            const float badge_w = badge_ts.x + badge_pad_x * 2.f;
            const float badge_h = badge_ts.y + badge_pad_y * 2.f;
            const float badge_round = badge_h * 0.5f;
            const ImRect badge_bb(
                { draw_item.Max.x - badge_w - ui_scale::s(10.f), draw_item.GetCenter().y - badge_h * 0.5f },
                { draw_item.Max.x - ui_scale::s(10.f), draw_item.GetCenter().y + badge_h * 0.5f });
            dl->AddRectFilled(badge_bb.Min, badge_bb.Max,
                ImGui::GetColorU32(ImVec4(colors::accent.x, colors::accent.y, colors::accent.z, 0.22f)),
                badge_round);
            dl->AddText(badge_font, badge_size,
                { badge_bb.GetCenter().x - badge_ts.x * 0.5f, badge_bb.GetCenter().y - badge_ts.y * 0.5f },
                ImGui::GetColorU32(colors::accent), badge);
        }

        if (pressed)
            s.selected = index;
    }

    float draw_config_cards(ImDrawList* dl, const ImVec2& origin, float width, config_page::state& s,
        bool& blocks_drag)
    {
        float y = origin.y;
        int visible = 0;

        for (int i = 0; i < s.count; ++i)
        {
            if (!matches_search(s.names[i], s.search))
            {
                ImGuiID id = ImGui::GetID((void*)(intptr_t)(i + 100));
                row_anim& anim = g_row_anims[id];
                anim.visible = ImLerp(anim.visible, 0.f, 1.f);
                continue;
            }

            const float item_h = ui_scale::s(layout::config_item_h);
            const ImRect item({ origin.x, y }, { origin.x + width, y + item_h });
            draw_config_card(dl, item, s, i, blocks_drag);
            y += item_h + ui_scale::s(layout::config_item_gap);
            ++visible;
        }

        if (visible == 0)
        {
            ImFont* empty_font = fonts::inter(12.f);
            if (empty_font)
            {
                const char* empty = s.search[0] ? "No configs match your search" : "No configs yet";
                const float empty_size = fonts::size(empty_font);
                const ImVec2 ts = empty_font->CalcTextSizeA(empty_size, FLT_MAX, 0.f, empty);
                dl->AddText(empty_font, empty_size,
                    { origin.x + (width - ts.x) * 0.5f, y + ui_scale::s(24.f) },
                    ImGui::GetColorU32(colors::text_muted), empty);
            }
            y += ui_scale::s(48.f);
        }

        return y;
    }

    bool draw_action_button(ImDrawList* dl, const char* id, const char* label, const ImRect& rect,
        bool& blocks_drag, bool accent = false, bool danger = false)
    {
        ImGuiID gid = ImGui::GetID(id);
        ImGui::ItemAdd(rect, gid);

        bool hovered = false, held = false;
        const bool pressed = ImGui::ButtonBehavior(rect, gid, &hovered, &held);
        if (hovered || held)
            blocks_drag = true;

        static std::map<ImGuiID, float> btn_hover;
        float& h = btn_hover[gid];
        h = ImLerp(h, hovered || held ? 1.f : 0.f, 1.f);

        ImVec4 bg = colors::control;
        ImVec4 text_col = colors::text;

        if (accent)
        {
            bg = ImLerp(
                ImVec4(colors::accent.x * 0.82f, colors::accent.y * 0.82f, colors::accent.z * 0.82f, 1.f),
                colors::accent,
                h);
            text_col = colors::text;
        }
        else if (danger)
        {
            bg = ImLerp(colors::control, ImVec4(colors::danger.x, colors::danger.y, colors::danger.z, 0.35f), h);
            text_col = ImLerp(colors::text, colors::danger, 0.55f + h * 0.45f);
        }
        else
        {
            bg = ImLerp(colors::control, colors::control_hover, h);
            text_col = ImLerp(colors::text_muted, colors::text, 0.65f + h * 0.35f);
        }

        dl->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32(bg), ui_scale::s(layout::control_round));

        if (ImFont* font = fonts::inter(12.f))
        {
            const float label_size = fonts::size(font);
            const ImVec2 ts = font->CalcTextSizeA(label_size, FLT_MAX, 0.f, label);
            dl->AddText(font, label_size,
                { rect.Min.x + (rect.GetWidth() - ts.x) * 0.5f, rect.GetCenter().y - ts.y * 0.5f },
                ImGui::GetColorU32(text_col), label);
        }

        return pressed;
    }

    void draw_section_label(ImDrawList* dl, const ImVec2& pos, const char* label)
    {
        ImFont* font = fonts::inter(11.f, true);
        if (!font) font = fonts::inter(11.f);
        if (font)
            dl->AddText(font, fonts::size(font), pos, ImGui::GetColorU32(colors::header_text), label);
    }

    void draw_profile_summary(ImDrawList* dl, const ImRect& box, config_page::state& s, bool& blocks_drag)
    {
        (void)blocks_drag;

        const float pad = ui_scale::s(layout::box_pad_x);
        const ImRect inner(
            { box.Min.x + pad, box.Min.y + ui_scale::s(layout::box_pad_y) },
            { box.Max.x - pad, box.Min.y + ui_scale::s(layout::box_pad_y + 72.f) });

        dl->AddRectFilled(inner.Min, inner.Max, ImGui::GetColorU32(colors::control), ui_scale::s(layout::config_card_round));

        ImFont* label_font = fonts::inter(11.f, true);
        if (!label_font) label_font = fonts::inter(11.f);
        ImFont* title_font = fonts::inter(13.f, true);
        if (!title_font) title_font = fonts::inter(13.f);
        ImFont* meta_font  = fonts::inter(11.f);

        if (ImFont* icon_font = fonts::icon(14.f))
        {
            const float icon_size = fonts::size(icon_font);
            const ImVec2 ts = icon_font->CalcTextSizeA(icon_size, FLT_MAX, 0.f, icons::folder);
            dl->AddText(icon_font, icon_size,
                { inner.Min.x + ui_scale::s(10.f), inner.Min.y + ui_scale::s(11.f) },
                ImGui::GetColorU32(colors::accent), icons::folder);
        }

        if (label_font)
            dl->AddText(label_font, fonts::size(label_font), { inner.Min.x + ui_scale::s(30.f), inner.Min.y + ui_scale::s(12.f) },
                ImGui::GetColorU32(colors::header_text), "Selected profile");

        if (title_font)
            dl->AddText(title_font, fonts::size(title_font), { inner.Min.x + ui_scale::s(10.f), inner.Min.y + ui_scale::s(30.f) },
                ImGui::GetColorU32(colors::text), s.names[s.selected]);

        if (meta_font)
        {
            char date_line[64];
            ImFormatString(date_line, IM_ARRAYSIZE(date_line), "Updated %s", s.dates[s.selected]);
            dl->AddText(meta_font, fonts::size(meta_font), { inner.Min.x + ui_scale::s(10.f), inner.Min.y + ui_scale::s(48.f) },
                ImGui::GetColorU32(colors::text_muted), date_line);
        }
    }

    float draw_name_field(ImDrawList* dl, const ImRect& field, config_page::state& s, bool& blocks_drag)
    {
        const bool mouse_on_field = ImGui::IsMouseHoveringRect(field.Min, field.Max, false);

        static float name_hover = 0.f;

        const ImVec4 bg = ImLerp(colors::control, colors::control_hover, name_hover);
        dl->AddRectFilled(field.Min, field.Max, ImGui::GetColorU32(bg), ui_scale::s(layout::control_round));

        ImGui::SetCursorScreenPos({ field.Min.x + ui_scale::s(8.f), field.Min.y + ui_scale::s(5.f) });
        ImGui::PushItemWidth(field.GetWidth() - ui_scale::s(16.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Text, colors::text);

        if (ImFont* font = fonts::inter(13.f))
            ImGui::PushFont(font);

        ImGui::InputTextWithHint("##cfg_new_name_input", "Enter name...", s.new_name, sizeof(s.new_name));

        const bool input_active = ImGui::IsItemActive();
        const bool input_hovered = ImGui::IsItemHovered();
        if (input_active || input_hovered || mouse_on_field)
            blocks_drag = true;

        name_hover = ImLerp(name_hover,
            (mouse_on_field || input_hovered || input_active) ? 1.f : 0.f,
            1.f);

        if (ImFont* font = fonts::inter(13.f))
            ImGui::PopFont();

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();

        return field.Max.y;
    }

    void draw_action_grid(ImDrawList* dl, const ImRect& box, config_page::state& s, bool& blocks_drag, float& content_bottom)
    {
        const float pad = ui_scale::s(layout::box_pad_x);
        const float top = box.Min.y + ui_scale::s(layout::box_pad_y + 82.f);
        const float gap = ui_scale::s(6.f);
        const float btn_w = (box.GetWidth() - pad * 2.f - gap) * 0.5f;
        const float btn_h = ui_scale::s(28.f);

        const ImRect load_bb({ box.Min.x + pad, top }, { box.Min.x + pad + btn_w, top + btn_h });
        const ImRect save_bb({ load_bb.Max.x + gap, top }, { load_bb.Max.x + gap + btn_w, top + btn_h });
        const ImRect dup_bb({ box.Min.x + pad, top + btn_h + gap },
            { box.Min.x + pad + btn_w, top + btn_h * 2.f + gap });
        const ImRect del_bb({ dup_bb.Max.x + gap, dup_bb.Min.y },
            { dup_bb.Max.x + gap + btn_w, dup_bb.Max.y });

        if (draw_action_button(dl, "cfg_load_btn", "Load", load_bb, blocks_drag, true))
            set_status(s, "Config loaded");

        if (draw_action_button(dl, "cfg_save_btn", "Save", save_bb, blocks_drag))
            set_status(s, "Config saved");

        if (draw_action_button(dl, "cfg_dup_btn", "Duplicate", dup_bb, blocks_drag))
        {
            if (s.count >= config_page::state::max_configs)
                set_status(s, "Config list is full");
            else
            {
                char copy_name[config_page::state::name_len];
                ImFormatString(copy_name, IM_ARRAYSIZE(copy_name), "%s copy", s.names[s.selected]);
                copy_str(s.names[s.count], config_page::state::name_len, copy_name);
                copy_str(s.dates[s.count], config_page::state::date_len, "Just now");
                s.selected = s.count;
                ++s.count;
                set_status(s, "Config duplicated");
            }
        }

        if (draw_action_button(dl, "cfg_del_btn", "Delete", del_bb, blocks_drag, false, true))
        {
            if (s.count <= 1)
                set_status(s, "Cannot delete last config");
            else
            {
                for (int i = s.selected; i < s.count - 1; ++i)
                {
                    copy_str(s.names[i], config_page::state::name_len, s.names[i + 1]);
                    copy_str(s.dates[i], config_page::state::date_len, s.dates[i + 1]);
                }
                --s.count;
                if (s.selected >= s.count)
                    s.selected = s.count - 1;
                set_status(s, "Config deleted");
            }
        }

        const float sep_y = del_bb.Max.y + ui_scale::s(10.f);
        dl->AddLine(
            { box.Min.x + pad, sep_y },
            { box.Max.x - pad, sep_y },
            ImGui::GetColorU32(colors::separator), ui_scale::s(layout::separator_h));

        draw_section_label(dl, { box.Min.x + pad, sep_y + ui_scale::s(10.f) }, "Create new");

        const ImRect name_field({ box.Min.x + pad, sep_y + ui_scale::s(28.f) },
            { box.Max.x - pad, sep_y + ui_scale::s(28.f) + ui_scale::s(layout::control_h) });
        draw_name_field(dl, name_field, s, blocks_drag);

        const ImRect create_bb({ box.Min.x + pad, name_field.Max.y + ui_scale::s(8.f) },
            { box.Max.x - pad, name_field.Max.y + ui_scale::s(8.f) + btn_h });
        if (draw_action_button(dl, "cfg_create_btn", "Create config", create_bb, blocks_drag, true))
        {
            if (s.new_name[0] == '\0')
                set_status(s, "Enter a config name");
            else if (name_exists(s, s.new_name))
                set_status(s, "Config already exists");
            else if (s.count >= config_page::state::max_configs)
                set_status(s, "Config list is full");
            else
            {
                copy_str(s.names[s.count], config_page::state::name_len, s.new_name);
                copy_str(s.dates[s.count], config_page::state::date_len, "Just now");
                s.selected = s.count;
                ++s.count;
                s.new_name[0] = '\0';
                set_status(s, "Config created");
            }
        }

        content_bottom = create_bb.Max.y;
    }
}

namespace config_page
{
    void draw(state& s, bool& blocks_window_drag)
    {
        s.status_alpha = ImLerp(s.status_alpha, 1.f, 1.f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 content_pos = settings_panel::content_origin();
        const float left_w  = settings_panel::column_left_width();
        const float right_w = settings_panel::column_right_width();

        char header[48];
        ImFormatString(header, IM_ARRAYSIZE(header), "Profiles  %d", s.count);
        settings_panel::draw_header(dl, content_pos, header);

        const float header_h = settings_panel::header_block_h(header);
        const ImVec2 search_pos = { content_pos.x, content_pos.y + header_h };
        const ImRect search_bar(search_pos, search_pos + ImVec2(left_w, ui_scale::s(layout::config_search_h)));
        draw_search_field(dl, search_bar, s, blocks_window_drag);

        const ImVec2 cards_origin = { content_pos.x, search_bar.Max.y + ui_scale::s(10.f) };
        const float cards_bottom = draw_config_cards(dl, cards_origin, left_w, s, blocks_window_drag);

        const ImVec2 right_pos = { content_pos.x + left_w + ui_scale::s(layout::column_gap), content_pos.y };
        settings_panel::draw_header(dl, right_pos, "Manage");
        const ImVec2 actions_pos = { right_pos.x, right_pos.y + settings_panel::header_block_h("Manage") };
        const ImRect actions_box(actions_pos, actions_pos + ImVec2(right_w, ui_scale::s(layout::config_manage_h)));
        dl->AddRectFilled(actions_box.Min, actions_box.Max, ImGui::GetColorU32(colors::box), ui_scale::s(layout::box_round));

        draw_profile_summary(dl, actions_box, s, blocks_window_drag);
        float manage_bottom = actions_box.Min.y;
        draw_action_grid(dl, actions_box, s, blocks_window_drag, manage_bottom);

        if (ImFont* font = fonts::inter(12.f))
        {
            const ImVec2 status_pos = { content_pos.x, cards_bottom + ui_scale::s(10.f) };
            dl->AddText(font, fonts::size(font), status_pos,
                ImGui::GetColorU32(ImVec4(colors::text_muted.x, colors::text_muted.y, colors::text_muted.z, s.status_alpha)),
                s.status);
        }

        const float bottom = ImMax(cards_bottom + ui_scale::s(28.f), manage_bottom + ui_scale::s(layout::content_pad_y));
        settings_panel::finalize_scroll(bottom);
    }
}
