#include "helpers/screenshot.h"
#include <GL/gl.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
namespace screenshot {
static state current;
static bool pending = false;
state& get() { return current; }
void begin_frame() { current.capture_frame = pending; current.bounds_valid = false; pending = false; }
void request() { pending = true; }
void set_bounds(ImVec2 min, ImVec2 max) { current.bounds_min=min; current.bounds_max=max; current.bounds_valid=true; }
bool save_png(const char* path, int width, int height, ImVec2 min, ImVec2 max, ImVec2 display) {
    if (display.x <= 0 || display.y <= 0 || width <= 0 || height <= 0) return false;
    int x=std::clamp(static_cast<int>(min.x*width/display.x),0,width);
    int y=std::clamp(static_cast<int>(min.y*height/display.y),0,height);
    int right=std::clamp(static_cast<int>(max.x*width/display.x),x,width);
    int bottom=std::clamp(static_cast<int>(max.y*height/display.y),y,height);
    int w=right-x,h=bottom-y;
    if (!w || !h) return false;
    std::vector<unsigned char> pixels(static_cast<size_t>(w)*h*4), flipped(pixels.size());
    GLint alignment; glGetIntegerv(GL_PACK_ALIGNMENT,&alignment); glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadPixels(x,height-bottom,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    glPixelStorei(GL_PACK_ALIGNMENT,alignment);
    for (int row=0;row<h;++row) std::copy_n(pixels.data()+static_cast<size_t>(h-1-row)*w*4,w*4,flipped.data()+static_cast<size_t>(row)*w*4);
    return stbi_write_png(path,w,h,4,flipped.data(),w*4)!=0;
}
bool save_requested(int width,int height) {
    if (!current.capture_frame || !current.bounds_valid) return false;
    std::error_code ec; std::filesystem::create_directories("screenshots",ec);
    auto stamp=std::chrono::system_clock::now().time_since_epoch().count();
    auto path=std::string("screenshots/portfolio8_")+std::to_string(stamp)+".png";
    bool ok=!ec && save_png(path.c_str(),width,height,current.bounds_min,current.bounds_max,ImGui::GetIO().DisplaySize);
    current.capture_frame=current.bounds_valid=false; return ok;
}
}
