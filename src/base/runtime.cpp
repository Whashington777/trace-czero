#include "game.hpp"
#include "app/gui.h"
#include "helpers/widgets.h"
#include "helpers/fonts.h"
#include "theme/colors.h"
#include "helpers/screenshot.h"
#include "pages/aim_page.h"
#include "pages/visuals_page.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#include <dlfcn.h>
#include <atomic>
#include <array>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cfloat>
#include <thread>
namespace {
using namespace trace;
struct Runtime {
 bool initialized{},menu{true},disabled{};SDL_GLContext context{};SDL_Window* window{};
 ImGuiContext* imgui{};std::thread::id owner{};unsigned frames{};
 SDL_bool relative{SDL_FALSE};SDL_bool grabbed{SDL_FALSE};
 std::chrono::steady_clock::time_point last_attempt{},started{};
 bool captured{},changing_input{},toggle_requested{},close_requested{};
 int saved_cursor{SDL_ENABLE};
 std::array<Uint8, SDL_NUM_SCANCODES> previous_keys{};
 bool keyboard_sampled{};
 std::chrono::steady_clock::time_point diagnostic_time{};
};
Runtime& state(){static Runtime s;return s;}
void hooked_move(float dt,UserCmd* cmd,int active){
 auto& g=game();if(g.original_move)g.original_move(dt,cmd,active);
 auto& s=state();if(!s.initialized||s.owner!=std::this_thread::get_id())return;
 ImGui::SetCurrentContext(s.imgui);g.update(!s.menu);int key=gui::aim_settings().keybind;
 bool held=key>=ImGuiKey_NamedKey_BEGIN&&key<ImGuiKey_NamedKey_END&&ImGui::IsKeyDown(static_cast<ImGuiKey>(key));
 g.aim(cmd,dt,active,s.menu,held);
}
void hooked_ref(RefParams* params){auto& g=game();if(g.original_ref)g.original_ref(params);if(state().owner==std::this_thread::get_id())g.refdef(params);}
void native_relative(SDL_bool mode) {
 static auto original=reinterpret_cast<int(*)(SDL_bool)>(dlsym(RTLD_NEXT,"SDL_SetRelativeMouseMode"));
 if(original)original(mode);
}
void menu_mode(bool open) {
 auto& s = state();
 if (open == s.menu) return;
 std::fprintf(stderr, "[trace] menu %s\n", open ? "open" : "closed");
 if (open) {
     s.relative = SDL_GetRelativeMouseMode();
     s.grabbed = SDL_GetWindowGrab(s.window);
     s.saved_cursor = SDL_ShowCursor(SDL_QUERY);
 }
 s.menu = open;
 s.changing_input = true;
 if (game().ready) {
     auto clear = slot<void(*)()>(game().callbacks, 12 * 4);
     auto callback = slot<void(*)()>(game().callbacks, (open ? 10 : 9) * 4);
     if (clear) clear();
     if (callback) callback();
 }
 native_relative(open ? SDL_FALSE : s.relative);
 SDL_SetWindowGrab(s.window, open ? SDL_FALSE : s.grabbed);
 SDL_ShowCursor(open ? SDL_ENABLE : s.saved_cursor);
 s.changing_input = false;
 if (s.imgui) {
     ImGui::SetCurrentContext(s.imgui);
     if (open) ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
     else ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
 }
}

void cleanup(){
 auto& s=state();if(!s.initialized)return;
 game().detach(hooked_move,hooked_ref);
 menu_mode(false);ImGui::SetCurrentContext(s.imgui);
 gui::shutdown();ImGui_ImplOpenGL3_Shutdown();ImGui_ImplSDL2_Shutdown();ImGui::DestroyContext(s.imgui);
 s.imgui=nullptr;s.initialized=false;
}
void draw_esp(){
 auto& g=game();auto& cfg=gui::visual_settings();auto dl=ImGui::GetBackgroundDrawList();auto size=ImGui::GetIO().DisplaySize;
 if(g.ready&&g.camera_valid&&g.local_index&&cfg.esp_enabled){
 for(int i=0;i<g.count;++i){auto& p=g.players[i];
 if(cfg.team_check&&(g.local_team<1||p.team<1||p.team==g.local_team))continue;
 if(cfg.visible_only&&!p.visible)continue;
 float metres=length(p.origin-g.eyes)/39.3701f;if(metres>cfg.max_distance)continue;
 Vec3 top=p.origin+Vec3{0,0,p.maxs.z},bottom=p.origin+Vec3{0,0,p.mins.z};
 float tx,ty,bx,by;if(!g.project(top,size.x,size.y,tx,ty)||!g.project(bottom,size.x,size.y,bx,by))continue;
 float h=by-ty;if(h<3||h>size.y*2)continue;float w=h*0.45f;
 if(bx+w/2<0||bx-w/2>size.x||by<0||ty>size.y)continue;
 ImVec2 min(bx-w/2,ty),max(bx+w/2,by);ImU32 color=p.visible?IM_COL32(181,146,255,255):IM_COL32(190,190,200,220);
 if(cfg.box_esp){dl->AddRect(min-ImVec2(1,1),max+ImVec2(1,1),IM_COL32(0,0,0,220),0.f,3.f);dl->AddRect(min,max,color);}
 if(cfg.name_esp){auto ts=ImGui::CalcTextSize(p.name);dl->AddText({bx-ts.x/2,ty-ts.y-3},color,p.name);}
 char distance[24];std::snprintf(distance,sizeof(distance),"%.0f m",metres);auto ts=ImGui::CalcTextSize(distance);dl->AddText({bx-ts.x/2,by+2},color,distance);
 if(cfg.tracers){ImVec2 start(size.x/2,cfg.tracer_origin==1?size.y/2:size.y);dl->AddLine(start,{bx,by},color);}
 }
 }
 auto center=ImVec2(size.x/2,size.y/2);
 if(g.ready&&g.camera_valid&&cfg.crosshair){dl->AddLine(center-ImVec2(4,0),center+ImVec2(4,0),IM_COL32_WHITE);dl->AddLine(center-ImVec2(0,4),center+ImVec2(0,4),IM_COL32_WHITE);}
 if(g.ready&&g.camera_valid&&cfg.fov_circle){float cone=std::clamp(gui::aim_settings().fov,1.f,85.f);float radius=size.x*.5f*std::tan(cone*.01745329252f)/std::tan(g.local_fov*.00872664626f);if(radius<size.x*2)dl->AddCircle(center,radius,IM_COL32(169,128,255,130),96);}
 if(cfg.watermark) {
     auto& io = ImGui::GetIO();
     ImFont* font = fonts::inter(13.f);
     if (!font) font = ImGui::GetFont();
     const float font_size = fonts::size(font);
     const float scale = font_size / 13.f;
     const float pad = 12.f * scale;
     const float gap = 12.f * scale;
     const float height = 34.f * scale;
     const char* connection = g.ready ? (g.camera_valid ? "LIVE" : "MENU") : "LOADING";
     if (g.status.find("Unsupported") != std::string::npos ||
         g.status.find("failed") != std::string::npos) connection = "UNAVAILABLE";
     char fps[24];
     std::snprintf(fps, sizeof(fps), "%.0f FPS", io.Framerate);
     char clock[8] = "--:--";
     const std::time_t now = std::time(nullptr);
     std::tm local{};
     if (localtime_r(&now, &local)) std::strftime(clock, sizeof(clock), "%H:%M", &local);
     auto text_width = [&](const char* text) {
         return font->CalcTextSizeA(font_size, FLT_MAX, 0.f, text).x;
     };
     const float brand_width = text_width("TRACE");
     const float status_width = text_width(connection);
     const bool show_details = size.x > 440.f * scale;
     const float details_width = show_details ? gap * 4.f + text_width(fps) + text_width(clock) : 0.f;
     const float width = pad * 2.f + brand_width + gap * 2.f + 6.f * scale + status_width + details_width;
     const ImVec2 min(ImMax(8.f, size.x - width - 16.f * scale), 16.f * scale);
     const ImVec2 max = min + ImVec2(width, height);
     auto foreground = ImGui::GetForegroundDrawList();
     foreground->AddRectFilled(min + ImVec2(0, 3.f * scale), max + ImVec2(0, 5.f * scale),
                              IM_COL32(0, 0, 0, 45), 10.f * scale);
     foreground->AddRectFilled(min, max, IM_COL32(19, 20, 26, 242), 9.f * scale);
     foreground->AddRect(min, max, IM_COL32(255, 255, 255, 20), 9.f * scale, 1.f);
     const ImU32 accent = ImGui::GetColorU32(colors::accent);
     foreground->AddLine(min + ImVec2(12.f * scale, 1.f),
                         {max.x - 12.f * scale, min.y + 1.f}, accent, 2.f * scale);
     const float baseline = min.y + (height - font_size) * .5f;
     float x = min.x + pad;
     auto text = [&](const char* value, ImU32 color) {
         foreground->AddText(font, font_size, {x, baseline}, color, value);
         x += text_width(value);
     };
     auto divider = [&]() {
         x += gap;
         foreground->AddLine({x, min.y + 11.f * scale}, {x, max.y - 11.f * scale},
                             IM_COL32(255, 255, 255, 35));
         x += gap;
     };
     text("TRACE", IM_COL32(239, 239, 246, 255));
     divider();
     const ImU32 status_color = g.ready ? IM_COL32(121, 216, 164, 255) : accent;
     foreground->AddCircleFilled({x + 2.f * scale, min.y + height * .5f}, 2.5f * scale, status_color, 12);
     x += 6.f * scale;
     text(connection, status_color);
     if (show_details) {
         divider();
         text(fps, IM_COL32(183, 186, 201, 255));
         divider();
         text(clock, IM_COL32(183, 186, 201, 255));
     }
 }
}

void render(SDL_Window* window){
 auto& s=state();if(s.disabled)return;
 if(!s.initialized){
 if(!SDL_GL_GetCurrentContext())return;
 
 int profile=0;SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,&profile);
 if(profile==SDL_GL_CONTEXT_PROFILE_CORE){s.disabled=true;std::fprintf(stderr,"[trace] Core-only GL context unsupported by this game profile\n");return;}
 s.context=SDL_GL_GetCurrentContext();s.window=window;s.owner=std::this_thread::get_id();
 IMGUI_CHECKVERSION();s.imgui=ImGui::CreateContext();ImGui::GetIO().IniFilename=nullptr;ImGui::GetIO().LogFilename=nullptr;ImGui::GetIO().ConfigInputTrickleEventQueue=false;
 if(!ImGui_ImplSDL2_InitForOpenGL(window,s.context)){ImGui::DestroyContext(s.imgui);s.disabled=true;return;}
 if(!ImGui_ImplOpenGL3_Init("#version 120")){ImGui_ImplSDL2_Shutdown();ImGui::DestroyContext(s.imgui);s.disabled=true;return;}
 gui::initialize();gui::aim_settings().ignore_team=true;gui::aim_settings().fov=5;gui::aim_settings().smoothing=0;gui::aim_settings().enabled=false;gui::visual_settings().esp_enabled=false;
 widgets::set_basic_mode(true);
 gui::visual_settings().visible_only=false;
 gui::visual_settings().health_bar=false;gui::visual_settings().hitmarker=false;
 s.initialized=true;s.started=std::chrono::steady_clock::now();s.menu=false;menu_mode(true);
 std::fprintf(stderr,"[trace] GUI initialized (i386); Insert menu, End detach\n");
 }
 if(s.owner!=std::this_thread::get_id()||window!=s.window||SDL_GL_GetCurrentContext()!=s.context)return;
 ImGui::SetCurrentContext(s.imgui);
 auto now=std::chrono::steady_clock::now();if(now-s.last_attempt>std::chrono::seconds(1)){s.last_attempt=now;auto before=game().status;game().attach(hooked_move,hooked_ref);
 if(before!=game().status)std::fprintf(stderr,"[trace] %s\n",game().status.c_str());
 if(game().ready)game().attach_studio();}
 screenshot::begin_frame();gui::prepare_frame();ImGui_ImplOpenGL3_NewFrame();ImGui_ImplSDL2_NewFrame();
 
 if (ImGui::GetIO().DisplaySize.x <= 0 || ImGui::GetIO().DisplaySize.y <= 0) return;
 ImGui::NewFrame();
 
 int key_count = 0;
 const Uint8* physical = SDL_GetKeyboardState(&key_count);
 const bool focused = SDL_GetKeyboardFocus() == s.window;
 auto edge = [&](SDL_Scancode code) {
     return focused && s.keyboard_sampled && physical && code < key_count &&
            physical[code] && !s.previous_keys[code];
 };
 const bool insert_edge = edge(SDL_SCANCODE_INSERT);
 const bool escape_edge = edge(SDL_SCANCODE_ESCAPE);
 const bool end_edge = edge(SDL_SCANCODE_END);
 if (physical) {
     std::copy_n(physical, std::min(key_count, int(SDL_NUM_SCANCODES)), s.previous_keys.begin());
     s.keyboard_sampled = true;
 }
 if(end_edge || ImGui::IsKeyPressed(ImGuiKey_End,false)){cleanup();s.disabled=true;return;}
 int key = gui::menu_key();
 bool toggle = !widgets::capturing_key() &&
     (s.toggle_requested || (key == ImGuiKey_Insert && insert_edge) ||
      (key != ImGuiKey_Insert && key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END &&
       ImGui::IsKeyPressed(static_cast<ImGuiKey>(key), false)));
 s.toggle_requested = false;
 if (toggle) menu_mode(!s.menu);
 if (s.close_requested || (s.menu && escape_edge && !widgets::capturing_key() && !ImGui::GetIO().WantTextInput)) {
     menu_mode(false);
 }
 s.close_requested = false;
 if (now - s.diagnostic_time > std::chrono::seconds(5)) {
     s.diagnostic_time = now;
     std::fprintf(stderr, "[trace] state connected=%d camera=%d local=%d players=%d aim=%d esp=%d menu=%d\n",
                  game().ready, game().camera_valid, game().local_index, game().count,
                  gui::aim_settings().enabled, gui::visual_settings().esp_enabled, s.menu);
 }
 draw_esp();if(s.menu)gui::render(window,false);if(gui::consume_close_request())menu_mode(false);
 ImGui::Render();ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());++s.frames;
 if(!s.captured && now-s.started>std::chrono::seconds(10)) {
 if(const char* path=std::getenv("TRACE_SCREENSHOT")) {
 int w,h;SDL_GL_GetDrawableSize(window,&w,&h);
 bool ok=screenshot::save_png(path,w,h,{0,0},ImGui::GetIO().DisplaySize,ImGui::GetIO().DisplaySize);
 std::fprintf(stderr,"[trace] capture=%d connected=%d camera=%d players=%d local=%d fov=%.0f\n",ok,game().ready,game().camera_valid,game().count,game().local_index,game().local_fov);
 }s.captured=true;
 }
 }
}
extern "C" unsigned tracecs16_rendered_frames(){return state().frames;}
extern "C" int tracecs16_connected(){return trace::game().ready?1:0;}
extern "C" void SDL_GL_SwapWindow(SDL_Window* window){
 static auto original=reinterpret_cast<void(*)(SDL_Window*)>(dlsym(RTLD_NEXT,"SDL_GL_SwapWindow"));
 render(window);if(original)original(window);
}
namespace {


thread_local bool fetching_event = false;
bool consume_event(SDL_Event* event) {
 auto& s = state();
 if (!event || !s.initialized || s.owner != std::this_thread::get_id()) return false;
 ImGui::SetCurrentContext(s.imgui);
 ImGui_ImplSDL2_ProcessEvent(event);
 if (event->type == SDL_KEYDOWN && !event->key.repeat && !widgets::capturing_key()) {
     bool insert = event->key.keysym.scancode == SDL_SCANCODE_INSERT || event->key.keysym.sym == SDLK_INSERT;
     bool escape = event->key.keysym.scancode == SDL_SCANCODE_ESCAPE || event->key.keysym.sym == SDLK_ESCAPE;
     if (insert && gui::menu_key() == ImGuiKey_Insert) s.toggle_requested = true;
     if (s.menu && escape && !ImGui::GetIO().WantTextInput) s.close_requested = true;
 }
 bool mouse = event->type == SDL_MOUSEMOTION || event->type == SDL_MOUSEBUTTONDOWN ||
              event->type == SDL_MOUSEBUTTONUP || event->type == SDL_MOUSEWHEEL;
 bool keyboard = event->type == SDL_KEYDOWN || event->type == SDL_KEYUP ||
                 event->type == SDL_TEXTINPUT || event->type == SDL_TEXTEDITING;
 return s.menu && (mouse || keyboard);
}
}
extern "C" int SDL_PollEvent(SDL_Event* event) {
 static auto original = reinterpret_cast<int(*)(SDL_Event*)>(dlsym(RTLD_NEXT, "SDL_PollEvent"));
 if (!original) return 0;
 if (fetching_event || !event) return original(event);
 for (;;) {
     fetching_event = true;
     int result = original(event);
     fetching_event = false;
     if (!result || !consume_event(event)) return result;
 }
}
extern "C" int SDL_WaitEventTimeout(SDL_Event* event, int timeout) {
 static auto original = reinterpret_cast<int(*)(SDL_Event*, int)>(dlsym(RTLD_NEXT, "SDL_WaitEventTimeout"));
 if (!original) return 0;
 int remaining = state().initialized && state().menu ? 0 : timeout;
 if (fetching_event || !event) return original(event, remaining);
 for (;;) {
     fetching_event = true;
     int result = original(event, remaining);
     fetching_event = false;
     if (!result || !consume_event(event)) return result;
     
     remaining = 0;
 }
}
extern "C" int SDL_WaitEvent(SDL_Event* event) {
 static auto original = reinterpret_cast<int(*)(SDL_Event*)>(dlsym(RTLD_NEXT, "SDL_WaitEvent"));
 static auto timed = reinterpret_cast<int(*)(SDL_Event*, int)>(dlsym(RTLD_NEXT, "SDL_WaitEventTimeout"));
 if (!original) return 0;
 bool nonblocking = state().initialized && state().menu;
 if (fetching_event || !event) return nonblocking && timed ? timed(event, 0) : original(event);
 for (;;) {
     fetching_event = true;
     int result = nonblocking && timed ? timed(event, 0) : original(event);
     fetching_event = false;
     if (!result || !consume_event(event)) return result;
     if (!timed) return 0;
     nonblocking = true;
 }
}
extern "C" void SDL_GL_DeleteContext(SDL_GLContext context){
 static auto original=reinterpret_cast<void(*)(SDL_GLContext)>(dlsym(RTLD_NEXT,"SDL_GL_DeleteContext"));
 if(state().initialized&&context==state().context&&state().owner==std::this_thread::get_id()){SDL_GL_MakeCurrent(state().window,context);cleanup();}
 if(original)original(context);
}

extern "C" int SDL_SetRelativeMouseMode(SDL_bool mode) {
 static auto original=reinterpret_cast<int(*)(SDL_bool)>(dlsym(RTLD_NEXT,"SDL_SetRelativeMouseMode"));
 auto& s=state();if(s.initialized&&s.menu){if(!s.changing_input)s.relative=mode;mode=SDL_FALSE;}
 return original?original(mode):-1;
}
