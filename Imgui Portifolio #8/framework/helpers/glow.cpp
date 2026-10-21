#include "glow.h"

#include "core/glow_renderer.h"
#include "theme/colors.h"

namespace glow
{
    void draw_accent_rect(ImDrawList* dl, const ImRect& rect, float round, float strength)
    {
#ifdef _WIN32
        glow_renderer::queue_rect(rect, round, strength, colors::accent);
#else
        glow_renderer::draw_rect(dl, rect, round, strength, colors::accent);
#endif
    }

    void clear_frame()
    {
        glow_renderer::clear_queue();
    }
}
