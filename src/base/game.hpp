#pragma once
#include "sdk.hpp"
#include "recoil.hpp"
#include "modules.hpp"
#include <array>
#include <chrono>
#include <string>
namespace trace {
struct HeadSample {
 Vec3 centre{},origin{};
 const void* entity_model{};
 int sequence{};
 std::chrono::steady_clock::time_point time{};
 bool valid{};
};
struct Game {
 Module engine,client;void* api{};void* callbacks{};const void* extra{};
 void* studio_api{};
 void (*original_draw_points)(){};
 void (*original_set_header)(void*){};
 void* current_studio_header{};
 std::array<HeadSample,33> heads{};
 bool studio_ready{};
 bool attach_studio();
 void detach_studio();
 void capture_head();
 MoveFn original_move{};RefFn original_ref{};RefParams camera{};
 std::array<Player,32> players{};int count{},local_index{},local_team{},target{};
 BurstShots burst{};
 Vec3 eyes{};bool camera_valid{},ready{};float local_fov{90};std::string status{"Waiting for engine"};
 std::chrono::steady_clock::time_point camera_time{};
 bool attach(MoveFn move,RefFn ref);void detach(MoveFn move,RefFn ref);
 void refdef(RefParams* params);void update(bool allow_traces = true);bool project(Vec3 point,float width,float height,float& x,float& y)const;
 bool visible(Vec3 point,int index)const;void aim(UserCmd* cmd,float dt,int active,bool menu_open,bool key_held);
};
Game& game();
}
