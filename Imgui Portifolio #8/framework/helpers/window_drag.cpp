#include "window_drag.h"

#include "imgui.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace window_drag
{
    namespace
    {
        struct drag_state
        {
            bool    active      = false;
            ImVec2  mouse_start = {};
            ImVec2  pos_start   = {};
        };

        drag_state g_panel_drag;
    }

    void handle_panel(ImVec2& panel_pos, const ImRect& drag_rect, bool block_drag)
    {
        if (block_drag)
        {
            g_panel_drag.active = false;
            return;
        }

        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const bool in_drag_rect = drag_rect.Contains(mouse);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && in_drag_rect)
        {
            g_panel_drag.active      = true;
            g_panel_drag.mouse_start = mouse;
            g_panel_drag.pos_start   = panel_pos;
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            g_panel_drag.active = false;

        if (!g_panel_drag.active)
            return;

        panel_pos.x = g_panel_drag.pos_start.x + (mouse.x - g_panel_drag.mouse_start.x);
        panel_pos.y = g_panel_drag.pos_start.y + (mouse.y - g_panel_drag.mouse_start.y);
    }

 #ifdef _WIN32
    void handle(HWND hwnd, const ImRect& drag_rect, bool block_drag)
    {
        if (!hwnd)
            return;

        static bool active = false;
        static POINT mouse_start = {};
        static POINT win_start   = {};

        if (block_drag)
        {
            active = false;
            return;
        }

        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const bool in_drag_rect = drag_rect.Contains(mouse);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && in_drag_rect)
        {
            active = true;
            ::GetCursorPos(&mouse_start);
            RECT rc = {};
            ::GetWindowRect(hwnd, &rc);
            win_start.x = rc.left;
            win_start.y = rc.top;
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            active = false;

        if (!active)
            return;

        POINT cur = {};
        ::GetCursorPos(&cur);
        ::SetWindowPos(hwnd, nullptr,
            win_start.x + (cur.x - mouse_start.x),
            win_start.y + (cur.y - mouse_start.y),
            0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
#endif
}
