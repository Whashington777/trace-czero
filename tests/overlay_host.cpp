#include <SDL.h>
#include <SDL_opengl.h>
#include <dlfcn.h>
#include <cstdio>
int main(int argc,char**){
 bool detach=argc>1;
 if(SDL_Init(SDL_INIT_VIDEO))return 1;
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
 auto window=SDL_CreateWindow("Trace overlay test",0,0,1280,900,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);
 if(!window){std::fprintf(stderr,"%s\n",SDL_GetError());return 1;}
 auto context=SDL_GL_CreateContext(window);if(!context)return 1;
 for(int i=0;i<30;++i){
 if(detach&&i==20) {SDL_Event end{};end.type=SDL_KEYDOWN;end.key.windowID=SDL_GetWindowID(window);end.key.keysym.sym=SDLK_END;end.key.keysym.scancode=SDL_SCANCODE_END;SDL_PushEvent(&end);}
 SDL_Event e;while(SDL_PollEvent(&e)){}glClearColor(.05,.05,.06,1);glClear(GL_COLOR_BUFFER_BIT);SDL_GL_SwapWindow(window);}
 auto count=reinterpret_cast<unsigned(*)()>(dlsym(RTLD_DEFAULT,"tracecs16_rendered_frames"));
 auto connected=reinterpret_cast<int(*)()>(dlsym(RTLD_DEFAULT,"tracecs16_connected"));
 bool ok=count&&(detach?(count()>=20&&count()<30):count()>=30)&&connected&&connected()==0;
 std::printf("%s: 32-bit preloaded overlay, %u frames; absent game modules rejected\n",ok?"PASS":"FAIL",count?count():0);
 SDL_GL_DeleteContext(context);SDL_DestroyWindow(window);SDL_Quit();return ok?0:1;
}
