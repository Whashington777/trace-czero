#include "game.hpp"
#include "build_profile.hpp"
#include "pages/aim_page.h"
#include "pages/visuals_page.h"
#include "app/gui.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
namespace trace {
Game& game(){static Game instance;return instance;}
namespace {
void studio_points_hook() {
 auto& g=game();
 g.capture_head();
 if(g.original_draw_points)g.original_draw_points();
}
void studio_header_hook(void* header) {
 auto& g=game();
 g.current_studio_header=header;
 if(g.original_set_header)g.original_set_header(header);
}
}
bool Game::attach_studio() {
 if(!ready)return false;
 studio_api=reinterpret_cast<void*>(client.base+profile::client_studio_api);
 auto points=slot<void(*)()>(studio_api,100);
 auto header=slot<void(*)(void*)>(studio_api,140);
 if(points==studio_points_hook&&header==studio_header_hook){studio_ready=true;return true;}
 if(reinterpret_cast<std::uintptr_t>(points)!=engine.base+profile::studio_draw_points ||
    reinterpret_cast<std::uintptr_t>(header)!=engine.base+profile::studio_set_header){studio_ready=false;return false;}
 original_draw_points=points;original_set_header=header;
 auto points_hook=&studio_points_hook;auto header_hook=&studio_header_hook;
 std::memcpy(static_cast<char*>(studio_api)+100,&points_hook,sizeof(points_hook));
 std::memcpy(static_cast<char*>(studio_api)+140,&header_hook,sizeof(header_hook));
 studio_ready=true;
 std::fprintf(stderr,"[trace] studio head-hitbox capture attached\n");
 return true;
}
void Game::detach_studio() {
 if(studio_api&&module("client.so").base==client.base) {
 if(slot<void(*)()>(studio_api,100)==studio_points_hook)
     std::memcpy(static_cast<char*>(studio_api)+100,&original_draw_points,sizeof(original_draw_points));
 if(slot<void(*)(void*)>(studio_api,140)==studio_header_hook)
     std::memcpy(static_cast<char*>(studio_api)+140,&original_set_header,sizeof(original_set_header));
 }
 studio_ready=false;current_studio_header=nullptr;
 for(auto& h:heads)h={};
}
void Game::capture_head() {
 if(!ready||!studio_ready||!studio_api||!current_studio_header)return;
 auto current=slot<void*(*)()>(studio_api,24);
 auto player_model=slot<void*(*)(int)>(studio_api,124);
 auto extradata=slot<void*(*)(void*)>(studio_api,16);
 auto bones=slot<const Matrix3x4*(*)()>(studio_api,64);
 if(!current||!player_model||!extradata||!bones)return;
 const void* entity=current();
 if(!entity||!read<int>(entity,4))return;
 int index=read<int>(entity,0);if(index<1||index>32)return;
 void* model=player_model(index-1);if(!model)return;
 const void* header=extradata(model);
 
 if(header!=current_studio_header||read<int>(header,0)!=0x54534449||read<int>(header,4)!=10)return;
 int length_bytes=read<int>(header,72),bone_count=read<int>(header,140);
 int hitbox_count=read<int>(header,156),hitbox_offset=read<int>(header,160);
 if(length_bytes<244||length_bytes>16*1024*1024||bone_count<1||bone_count>128||
    hitbox_count<1||hitbox_count>256||hitbox_offset<244||hitbox_offset>length_bytes||
    hitbox_count>(length_bytes-hitbox_offset)/32)return;
 const Matrix3x4* transform=bones();if(!transform)return;
 for(int i=0;i<hitbox_count;++i) {
 const auto* box=static_cast<const char*>(header)+hitbox_offset+i*32;
 if(read<int>(box,4)!=1)continue; 
 int bone=read<int>(box,0);if(bone<0||bone>=bone_count)continue;
 Vec3 min=read<Vec3>(box,8),max=read<Vec3>(box,20);
 if(!finite(min)||!finite(max)||min.x>max.x||min.y>max.y||min.z>max.z)continue;
 Vec3 centre=transform_point((min+max)*.5f,transform[bone]);
 Vec3 origin=read<Vec3>(entity,0xb48);
 if(!finite(centre)||!finite(origin)||trace::length(centre-origin)>128.f)continue;
 heads[index]={centre,origin,read<const void*>(entity,0xb94),read<int>(entity,0x2b0+12),std::chrono::steady_clock::now(),true};
 return;
 }
 heads[index].valid=false;
}

bool Game::attach(MoveFn move,RefFn ref){
 if(ready) {
     auto loaded_engine = module("hw.so"), loaded_client = module("client.so");
     if (loaded_engine.base == engine.base && loaded_client.base == client.base &&
         slot<MoveFn>(callbacks, create_move_slot) == move && slot<RefFn>(callbacks, refdef_slot) == ref) return true;
     ready = false;
     studio_ready = false;
     for (auto& sample : heads) sample = {};
     camera_valid = false;
     count = 0;
     target = 0;
 }
 auto e=module("hw.so"),c=module("client.so");if(!e.base||!c.base)return false;
 if(e.build_id!=profile::engine_id||c.build_id!=profile::client_id){
 status="Unsupported game build (no callbacks changed)";return false;}
 engine=e;client=c;api=reinterpret_cast<void*>(e.base+profile::engine_api);callbacks=reinterpret_cast<void*>(e.base+profile::client_callbacks);extra=reinterpret_cast<void*>(c.base+profile::extra_players);
 auto local=slot<void*(*)()>(api,0xcc);
 original_move=slot<MoveFn>(callbacks,create_move_slot);original_ref=slot<RefFn>(callbacks,refdef_slot);
 if(reinterpret_cast<std::uintptr_t>(local)!=e.base+profile::get_local_player || reinterpret_cast<std::uintptr_t>(original_move)!=c.base+profile::create_move || reinterpret_cast<std::uintptr_t>(original_ref)!=c.base+profile::calc_refdef){status="Callback/API verification failed";return false;}
 if(!executable(c,reinterpret_cast<std::uintptr_t>(original_move))||!executable(c,reinterpret_cast<std::uintptr_t>(original_ref))){status="Invalid callback addresses";return false;}
 std::memcpy(static_cast<char*>(callbacks)+create_move_slot,&move,sizeof(move));std::memcpy(static_cast<char*>(callbacks)+refdef_slot,&ref,sizeof(ref));
 ready=true;status="Connected";return true;
}
void Game::detach(MoveFn move,RefFn ref){
 detach_studio();
 burst={};
 if(ready&&module("hw.so").base==engine.base){
 if(slot<MoveFn>(callbacks,create_move_slot)==move)std::memcpy(static_cast<char*>(callbacks)+create_move_slot,&original_move,sizeof(original_move));
 if(slot<RefFn>(callbacks,refdef_slot)==ref)std::memcpy(static_cast<char*>(callbacks)+refdef_slot,&original_ref,sizeof(original_ref));}
 ready=false;camera_valid=false;count=0;target=0;status="Detached";
}
void Game::refdef(RefParams* p){
 if(!p)return;
 camera=*p;camera_time=std::chrono::steady_clock::now();
 camera_valid=finite(p->origin)&&finite(p->angles)&&p->maxclients>0&&p->maxclients<=32&&!p->intermission&&!p->spectator&&!p->demoplayback&&p->health>0;
 eyes=p->simorg+p->viewheight;
}
bool Game::visible(Vec3 point,int index)const{
 auto ev=slot<const void*>(api,0x150);if(!ev)return false;
 auto setup=slot<void(*)(int,int)>(ev,40);auto push=slot<void(*)()>(ev,44);auto pop=slot<void(*)()>(ev,48);
 auto solid=slot<void(*)(int)>(ev,52);auto hull=slot<void(*)(int)>(ev,56);auto trace=slot<void(*)(float*,float*,int,int,TraceResult*)>(ev,60);auto idx=slot<int(*)(TraceResult*)>(ev,32);
 if(!setup||!push||!pop||!solid||!hull||!trace||!idx)return false;
 TraceResult hit{};auto start=eyes;setup(0,1);push();solid(local_index-1);hull(2);
 trace(&start.x,&point.x,0,-1,&hit);int entity=idx(&hit);pop();
 if(entity>0&&entity!=index)return false;
 return !hit.startsolid&&!hit.allsolid&&(hit.fraction>=1.f||entity==index);
}
void Game::update(bool allow_traces){
 count=0;local_index=0;local_team=0;target=0;
 if(!ready||!camera_valid||std::chrono::steady_clock::now()-camera_time>std::chrono::milliseconds(500)){camera_valid=false;return;}
 auto getlocal=slot<void*(*)()>(api,0xcc);auto getentity=slot<void*(*)(int)>(api,0xd4);auto maxclients=slot<int(*)()>(api,0x90);auto playerinfo=slot<void(*)(int,PlayerInfo*)>(api,0x54);
 if(!getlocal||!getentity||!maxclients||!playerinfo)return;
 const void* local=getlocal();if(!local)return;local_index=read<int>(local,0);if(local_index<1||local_index>32){local_index=0;return;}
 local_team=read<short>(extra,local_index*116+42);
 local_fov=read<int>(reinterpret_cast<void*>(client.base+profile::hud),0x54);if(local_fov<1||local_fov>179)local_fov=90;
 const bool need_visibility = allow_traces && (gui::aim_settings().enabled ||
     (gui::visual_settings().esp_enabled && gui::visual_settings().visible_only));
 int limit=std::clamp(maxclients(),0,32);int sequence=read<int>(local,0x2b0+12);
 for(int i=1;i<=limit;++i){
 if(i==local_index)continue;
 const void* ent=getentity(i);
 if(!ent||!read<int>(ent,4)||read<int>(ent,0)!=i||!read<void*>(ent,0xb94)||read<int>(ent,0x2b0+12)<sequence)continue;
 int movetype=read<int>(ent,0x2b0+88);if(movetype==0||movetype==6||read<bool>(extra,i*116+60))continue;
 Player p{};p.index=i;p.alive=true;p.team=read<short>(extra,i*116+42);p.origin=read<Vec3>(ent,0xb48);
 if(!finite(p.origin))continue;
 p.mins=read<Vec3>(ent,0x2b0+124);p.maxs=read<Vec3>(ent,0x2b0+136);
 if(!finite(p.mins)||!finite(p.maxs)||p.maxs.z-p.mins.z<10||p.maxs.z-p.mins.z>100){bool duck=read<int>(ent,0x2b0+200)==1;p.mins={-16,-16,duck?-18.f:-36.f};p.maxs={16,16,duck?18.f:36.f};}
 const auto& head=heads[i];
 p.head_valid=head.valid && head.entity_model==read<const void*>(ent,0xb94) &&
     head.sequence==read<int>(ent,0x2b0+12) &&
     std::chrono::steady_clock::now()-head.time<std::chrono::milliseconds(100) &&
     length(p.origin-head.origin)<8.f;
 
 p.head=p.head_valid?head.centre:p.origin+Vec3{0,0,p.maxs.z-8.f};
 p.visible=need_visibility && visible(p.head,i);
 PlayerInfo info{};playerinfo(i,&info);if(info.name)std::snprintf(p.name,sizeof(p.name),"%.63s",info.name);else std::snprintf(p.name,sizeof(p.name),"Player %d",i);
 players[count++]=p;
 }
}
bool Game::project(Vec3 p,float w,float h,float& x,float& y)const{
 if(!ready||!finite(p)||w<=0||h<=0)return false;
 auto tri=slot<const void*>(api,0x148);auto fn=slot<int(*)(float*,float*)>(tri,48);if(!fn)return false;
 Vec3 screen{};if(fn(&p.x,&screen.x)!=0||!finite(screen))return false;
 x=(screen.x+1.f)*0.5f*w;y=(1.f-screen.y)*0.5f*h;return std::isfinite(x)&&std::isfinite(y);
}
void Game::aim(UserCmd* cmd,float dt,int active,bool menu_open,bool key_held){
 target=0;auto& cfg=gui::aim_settings();
 const bool firing=cmd&&active&&camera_valid&&!menu_open&&(cmd->buttons&1);
 const int event=ready?read<int>(reinterpret_cast<void*>(client.base+profile::local_shot_events),0):burst.previous_event;
 const int weapon=ready?read<int>(reinterpret_cast<void*>(client.base+profile::current_weapon),0):burst.weapon;
 burst.observe(event,weapon,firing);
 const bool control=cfg.recoil_enabled&&burst.active_from(cfg.recoil_start_bullet)&&finite(camera.punch);
 const Vec3 recoil=control?camera.punch*(2.f*std::clamp(cfg.recoil_control,0.f,100.f)/100.f):Vec3{};
 if(!cmd||!active||menu_open||!camera_valid||!local_index||!cfg.enabled||!finite(cmd->view)||!std::isfinite(dt)||dt<=0)return;
 if(cfg.keybind? !key_held : !(cmd->buttons&1))return;
 float best=std::clamp(cfg.fov,1.f,90.f);const Player* chosen=nullptr;Vec3 wanted{};
 for(int i=0;i<count;++i){auto& p=players[i];
 if(!p.head_valid||!p.visible||(cfg.ignore_team&&(local_team<1||p.team<1||p.team==local_team)))continue;
 if(length(p.head-eyes)>cfg.max_distance*39.3701f)continue; 
 auto a=angles(p.head-eyes);auto d=delta(a,cmd->view+recoil);float fov=std::hypot(d.x,d.y);
 if(std::isfinite(fov)&&fov<best){best=fov;chosen=&p;wanted=a;}}
 if(!chosen)return;
 auto old=cmd->view;
 cmd->view=smooth(old+recoil,wanted,cfg.smoothing,dt)-recoil;
 cmd->view.x=std::clamp(cmd->view.x,-89.f,89.f);cmd->view.y=yaw(cmd->view.y);cmd->view.z=0;
 fix_movement(old.y,cmd->view.y,cmd->forward,cmd->side);
 auto set=slot<void(*)(float*)>(api,0x8c);if(set)set(&cmd->view.x);target=chosen->index;
}
}
