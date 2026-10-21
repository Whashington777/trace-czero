#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"
#include "app/gui.h"
#include "helpers/widgets.h"
#include "pages/aim_page.h"
#include "helpers/screenshot.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
int main(int argc,char** argv) {
    int frame_limit=0,page=1; float scale=100.f; const char* output=nullptr; bool self_test=false; bool tests_ok=true;
    for(int i=1;i<argc;++i) {
        if(!std::strcmp(argv[i],"--self-test")) { self_test=true;frame_limit=70; }
        else if(!std::strcmp(argv[i],"--frames") && i+1<argc) frame_limit=std::atoi(argv[++i]);
        else if(!std::strcmp(argv[i],"--screenshot") && i+1<argc) output=argv[++i];
        else if(!std::strcmp(argv[i],"--page") && i+1<argc) page=std::atoi(argv[++i]);
        else if(!std::strcmp(argv[i],"--scale") && i+1<argc) scale=std::strtof(argv[++i],nullptr);
        else {std::fprintf(stderr,"Usage: %s [--frames N] [--screenshot file.png] [--page 0..3] [--scale 75..150] [--self-test]\n",argv[0]);return 2;}
    }
    if(output && !frame_limit) frame_limit=12;
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0) {std::fprintf(stderr,"SDL: %s\n",SDL_GetError());return 1;}
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2); SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
    auto window=SDL_CreateWindow("VELARIS / Linux preview",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1280,900,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI|((output||self_test)?SDL_WINDOW_HIDDEN:0));
    if(!window) {std::fprintf(stderr,"Window: %s\n",SDL_GetError());SDL_Quit();return 1;}
    auto context=SDL_GL_CreateContext(window);
    if(!context) {std::fprintf(stderr,"OpenGL: %s\n",SDL_GetError());SDL_DestroyWindow(window);SDL_Quit();return 1;}
    SDL_GL_MakeCurrent(window,context); SDL_GL_SetSwapInterval(1);
    std::fprintf(stdout,"Dear ImGui %s; OpenGL %s; pointer size %zu\n",IMGUI_VERSION,glGetString(GL_VERSION),sizeof(void*));
    IMGUI_CHECKVERSION();ImGui::CreateContext();
    ImGui::GetIO().IniFilename=nullptr; ImGui::GetIO().LogFilename=nullptr;
    bool platform=ImGui_ImplSDL2_InitForOpenGL(window,context);
    bool renderer=platform && ImGui_ImplOpenGL2_Init();
    if(!renderer) {if(platform)ImGui_ImplSDL2_Shutdown();ImGui::DestroyContext();SDL_GL_DeleteContext(context);SDL_DestroyWindow(window);SDL_Quit();return 1;}
    gui::initialize(); gui::select_page(page);gui::set_scale(scale);
    bool done=false,visible=true,save_ok=true;int frame=0;
    while(!done) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(event.type==SDL_QUIT || (event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_CLOSE))done=true;
            if(event.type==SDL_KEYDOWN && !event.key.repeat) {
                if(event.key.keysym.sym==SDLK_ESCAPE && !gui::wants_keyboard())done=true;

            }
        }
        if(done)break;
        if(SDL_GetWindowFlags(window)&SDL_WINDOW_MINIMIZED) {SDL_Delay(10);continue;}
        screenshot::begin_frame();gui::prepare_frame();
        ImGui_ImplOpenGL2_NewFrame();ImGui_ImplSDL2_NewFrame();
        if(self_test) {
            auto& io=ImGui::GetIO();io.DeltaTime=1.f/60.f;
            ImVec2 p((io.DisplaySize.x-700.f)/2.f,(io.DisplaySize.y-430.f)/2.f);
            auto click=[&](float x,float y,bool down) {io.AddMousePosEvent(p.x+x,p.y+y);io.AddMouseButtonEvent(0,down);};
            if(frame==5)click(200,55,true);
            if(frame==6)click(200,55,false);
            if(frame==10 || frame==30)click(300,178,true);
            if(frame==11 || frame==31)click(300,178,false);
            if(frame==25)io.AddKeyEvent(ImGuiKey_K,true);
            if(frame==26)io.AddKeyEvent(ImGuiKey_K,false);
            if(frame==45)io.AddKeyEvent(ImGuiKey_Escape,true);
            if(frame==46)io.AddKeyEvent(ImGuiKey_Escape,false);
            if(frame==50)click(275,96,true);
            if(frame==51)io.AddMousePosEvent(p.x+320,p.y+96);
            if(frame==52)click(320,96,false);
            auto check=[&](bool ok,const char* name) {std::printf("%s: %s\n",ok?"PASS":"FAIL",name);tests_ok &= ok;};
            if(frame==8)check(gui::aim_settings().enabled,"mouse toggle");
            if(frame==28)check(gui::aim_settings().keybind==ImGuiKey_K,"keyboard binding");
            if(frame==48)check(gui::aim_settings().keybind==0,"Escape clears binding");
            if(frame==54)check(gui::aim_settings().smoothing!=50.f,"slider drag");
            if(frame==55)gui::set_scale(150.f);
            if(frame==60)gui::set_scale(75.f);
        }
        ImGui::NewFrame();
        if(!widgets::capturing_key() && gui::menu_key()>=ImGuiKey_NamedKey_BEGIN && ImGui::IsKeyPressed(static_cast<ImGuiKey>(gui::menu_key()),false))visible=!visible;
        if(visible)gui::render(window);
        ImGui::Render();
        int w,h;SDL_GL_GetDrawableSize(window,&w,&h);glViewport(0,0,w,h);
        if(screenshot::get().capture_frame) glClearColor(0,0,0,0);
        else glClearColor(0.055f,0.055f,0.07f,1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        screenshot::save_requested(w,h);
        ++frame;
        if(output && frame>=frame_limit) save_ok=screenshot::save_png(output,w,h,{0,0},ImGui::GetIO().DisplaySize,ImGui::GetIO().DisplaySize);
        SDL_GL_SwapWindow(window);
        done=gui::wants_close() || (frame_limit>0 && frame>=frame_limit);
    }
    gui::shutdown();ImGui_ImplOpenGL2_Shutdown();ImGui_ImplSDL2_Shutdown();ImGui::DestroyContext();
    SDL_GL_DeleteContext(context);SDL_DestroyWindow(window);SDL_Quit();
    if(!save_ok)std::fprintf(stderr,"Screenshot write failed\n");
    return save_ok && tests_ok?0:1;
}
