#pragma once

#ifdef _WIN32
#include <windows.h>
#else
struct SDL_Window;
using HWND = SDL_Window*;
#endif

struct ID3D11Device;

namespace aim_page { struct state; }
namespace visuals_page { struct state; }
namespace config_page { struct state; }
namespace settings_page { struct state; }

namespace gui
{
    void initialize(ID3D11Device* device = nullptr);
    void shutdown();
    bool wants_close();
    bool consume_close_request();
    bool wants_keyboard();
    int menu_key();
    aim_page::state& aim_settings();
    visuals_page::state& visual_settings();
    config_page::state& configurations();
    settings_page::state& menu_settings();
    void select_page(int index);
    void set_scale(float percent);
    void prepare_frame();
    void render(HWND hwnd, bool draw_backdrop = true);
}
