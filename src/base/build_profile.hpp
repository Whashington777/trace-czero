#pragma once
#include <cstdint>
namespace trace::profile {


inline constexpr char engine_id[]="2eb3ff7acb804319d453335feed3be5ed21faab3";
inline constexpr char client_id[]="3d9e527b99dd5455cb39693752401bedf6790f6b";
inline constexpr std::uintptr_t engine_api=0x2beb00;
inline constexpr std::uintptr_t client_callbacks=0xf77f80;
inline constexpr std::uintptr_t extra_players=0x1f9420;
inline constexpr std::uintptr_t client_studio_api=0x1fec80;
inline constexpr std::uintptr_t studio_draw_points=0xd0980;
inline constexpr std::uintptr_t studio_set_header=0xc63b0;
inline constexpr std::uintptr_t local_shot_events=0x1e6b84;
inline constexpr std::uintptr_t current_weapon=0x1f06d4;
inline constexpr std::uintptr_t hud=0x1e9900;
inline constexpr std::uintptr_t get_local_player=0x156d60;
inline constexpr std::uintptr_t create_move=0xc1b40;
inline constexpr std::uintptr_t calc_refdef=0xd38d0;
}
