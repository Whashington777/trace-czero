#include "base/game.hpp"
#include "app/gui.h"
#include "pages/visuals_page.h"
#include <cstdio>
#include <cstdlib>
#include <array>
using namespace trace;
std::array<char,3000> local{},enemy{};int push_count{},pop_count{};
template<class T> void put(void* p,std::size_t offset,T value){std::memcpy(static_cast<char*>(p)+offset,&value,sizeof(value));}
void check(bool ok,const char* label){if(!ok){std::fprintf(stderr,"FAIL %s\n",label);std::exit(1);}}
int main(){
 gui::visual_settings().esp_enabled=true;gui::visual_settings().visible_only=true;
 char api[540]{},extra[116*33]{},hud[100]{},ev[88]{},tri[80]{};
 put(local.data(),0,1);put(local.data(),0x2b0+12,20);
 put(enemy.data(),0,2);put(enemy.data(),4,1);put(enemy.data(),0xb94,reinterpret_cast<void*>(1));
 put(enemy.data(),0x2b0+12,20);put(enemy.data(),0x2b0+88,3);
 put(enemy.data(),0xb48,Vec3{1000,0,0});put(enemy.data(),0x2b0+124,Vec3{-16,-16,-36});put(enemy.data(),0x2b0+136,Vec3{16,16,36});
 put(extra,116+42,static_cast<short>(1));put(extra,232+42,static_cast<short>(2));put(hud,0x54,90);
 put(api,0xcc,+[]()->void*{return local.data();});put(api,0xd4,+[](int i)->void*{return i==2?enemy.data():nullptr;});
 put(api,0x90,+[](){return 2;});put(api,0x54,+[](int,PlayerInfo* p){p->name="Fixture player";});put(api,0x150,static_cast<void*>(ev));
 put(ev,40,+[](int,int){});put(ev,44,+[](){++push_count;});put(ev,48,+[](){++pop_count;});put(ev,52,+[](int){});put(ev,56,+[](int){});
 put(ev,60,+[](float*,float*,int,int,TraceResult* result){result->fraction=1.f;result->ent=2;});put(ev,32,+[](TraceResult* result){return result->ent;});
 put(api,0x148,static_cast<void*>(tri));put(tri,48,+[](float* in,float* out){out[0]=in[0];out[1]=in[1];out[2]=0;return in[2]<0?1:0;});
 Game g;g.api=api;g.extra=extra;g.client.base=reinterpret_cast<std::uintptr_t>(hud)-0x1e9900;g.ready=g.camera_valid=true;g.camera_time=std::chrono::steady_clock::now();
 g.update();check(g.count==1&&g.local_index==1&&g.local_team==1,"entity snapshot and local team");
 check(g.players[0].visible&&g.players[0].head.z==28&&g.local_fov==90,"trace, approximate upper body, FOV");
 check(push_count==pop_count&&push_count==1,"prediction trace state restored");
 float x,y;check(g.project({0,0,1},1280,720,x,y)&&x==640&&y==360,"NDC centre maps to screen centre");
 check(g.project({1,1,1},1280,720,x,y)&&x==1280&&y==0,"NDC axes and screen orientation");
 check(!g.project({0,0,-1},1280,720,x,y),"behind-camera projection rejected");
 put(extra,232+60,true);g.update();check(g.count==0,"dead player snapshot rejected");put(extra,232+60,false);
 put(enemy.data(),0x2b0+12,19);g.update();check(g.count==0,"stale entity rejected");
 g.camera_time=std::chrono::steady_clock::now()-std::chrono::seconds(1);g.update();check(!g.camera_valid&&g.count==0,"stale camera invalidates snapshot");
 std::puts("PASS snapshot, trace state, FOV, projection, death, stale entity/camera");
}
