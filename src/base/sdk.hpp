#pragma once
#include "math.hpp"
#include <cstdint>
#include <cstring>
#include <cstddef>
namespace trace {
static_assert(sizeof(void*)==4,"This adapter targets i386 GoldSrc only");
template<class T> T read(const void* p,std::size_t offset){T v{};if(p)std::memcpy(&v,static_cast<const char*>(p)+offset,sizeof(v));return v;}
template<class T> T slot(const void* p,std::size_t offset){return read<T>(p,offset);}
struct UserCmd {
 std::int16_t lerp;std::uint8_t msec,pad;Vec3 view;
 float forward,side,up;std::uint8_t light,pad2;std::uint16_t buttons;
 std::uint8_t impulse,weapon;std::uint16_t pad3;int impact;Vec3 impact_pos;
};
static_assert(sizeof(UserCmd)==52 && offsetof(UserCmd,buttons)==30);
struct RefParams {
 Vec3 origin,angles,forward,right,up;float frametime,time;
 int intermission,paused,spectator,onground,waterlevel;
 Vec3 velocity,simorg,viewheight;float idealpitch;Vec3 cl_angles;
 int health;Vec3 crosshair;float viewsize;Vec3 punch;
 int maxclients,viewentity,playernum,max_entities,demoplayback,hardware,smoothing;
 std::uint32_t cmd,movevars;int viewport[4];int next_view,only_client;
};
static_assert(sizeof(RefParams)==232 && offsetof(RefParams,punch)==0xa0);
struct PlayerInfo {const char* name;short ping;std::uint8_t local,spectator,loss;std::uint8_t pad[3];const char* model;short top,bottom;std::uint64_t steamid;};
static_assert(sizeof(PlayerInfo)==28 && offsetof(PlayerInfo,steamid)==20);
struct TraceResult {int allsolid,startsolid,inopen,inwater;float fraction;Vec3 end;Vec3 plane_normal;float plane_dist;int ent;Vec3 delta;int hitgroup;};
static_assert(sizeof(TraceResult)==68);
struct Player {int index{},team{};Vec3 origin{},head{},mins{},maxs{};bool visible{},alive{},head_valid{};char name[64]{};};
using MoveFn=void(*)(float,UserCmd*,int);
using RefFn=void(*)(RefParams*);
constexpr std::size_t create_move_slot=14*4,refdef_slot=19*4;
}
