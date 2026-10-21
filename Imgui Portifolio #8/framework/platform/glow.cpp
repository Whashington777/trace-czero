#include "core/glow_renderer.h"
#include <cmath>
namespace glow_renderer {
bool init(ID3D11Device*) { return true; }
void shutdown() {}
void clear_queue() {}
void flush(ID3D11DeviceContext*, const ImVec2&) {}
void draw_rect(ImDrawList* dl, const ImRect& rect, float round, float strength, const ImVec4& color) {
    
    for (int i = 14; i >= 1; --i) {
        const float spread = static_cast<float>(i);
        ImVec4 tint = color;
        tint.w *= strength * 0.045f * std::exp(-spread * spread / 90.f);
        dl->AddRect(rect.Min - ImVec2(spread, spread), rect.Max + ImVec2(spread, spread),
                    ImGui::GetColorU32(tint), round + spread, 2.f);
    }
}
void queue_rect(const ImRect& r, float round, float strength, const ImVec4& color) {
    draw_rect(ImGui::GetBackgroundDrawList(), r, round, strength, color);
}
}
