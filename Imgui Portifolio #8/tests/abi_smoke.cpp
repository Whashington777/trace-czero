#include "app/gui.h"
#include "imgui.h"
#include <cstdio>
int main() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    gui::initialize(); gui::prepare_frame();
    const bool ok = ImGui::GetIO().Fonts->Fonts.Size > 0;
    std::printf("%s: ImGui %s, %zu-bit GUI, %d fonts initialized\n", ok?"PASS":"FAIL",IMGUI_VERSION,sizeof(void*)*8,ImGui::GetIO().Fonts->Fonts.Size);
    gui::shutdown(); ImGui::DestroyContext();
    return ok?0:1;
}
