#pragma once

#ifdef _WIN32
#include <d3d11.h>
#endif
#include "imgui.h"

namespace screenshot
{
    struct state
    {
        bool   capture_frame = false;
        bool   bounds_valid  = false;
        ImVec2 bounds_min    = {};
        ImVec2 bounds_max    = {};
    };

    state& get();

    void begin_frame();
    void request();
    void set_bounds(ImVec2 min, ImVec2 max);

#ifdef _WIN32
    bool capture_and_save(ID3D11Device* device, ID3D11DeviceContext* ctx,
        ID3D11RenderTargetView* rtv, ImVec2 bounds_min, ImVec2 bounds_max);
#else
    bool save_png(const char* path, int width, int height, ImVec2 min, ImVec2 max, ImVec2 display);
    bool save_requested(int width, int height);
#endif
}
