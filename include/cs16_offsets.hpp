#pragma once
#include <cstddef>
#include <cstdint>


namespace cs16 {
namespace windows_client {
inline constexpr std::uintptr_t V_CalcRefdef = 0x6d610;
inline constexpr std::uintptr_t V_CalcNormalRefdef = 0x6ab20;
inline constexpr std::uintptr_t gEngfuncs = 0x117920;
inline constexpr std::uintptr_t gHUD = 0x117b48;
inline constexpr std::uintptr_t camera_origin_snapshot = 0x134b04;
inline constexpr std::uintptr_t camera_angles_snapshot = 0x134af8;
inline constexpr std::uintptr_t hud_player_origin = 0x117b80;
inline constexpr std::uintptr_t hud_player_angles = 0x117b8c;
inline constexpr std::uintptr_t hud_fov_int = 0x117ba0;
inline constexpr std::uintptr_t last_fov_float = 0x1275b0;
}
namespace linux_client {
inline constexpr std::uintptr_t V_CalcRefdef = 0xd38d0;
inline constexpr std::uintptr_t gEngfuncs = 0x1f0480;
inline constexpr std::uintptr_t gHUD = 0x1e9900;
inline constexpr std::uintptr_t v_origin = 0x204e10;
inline constexpr std::uintptr_t v_angles = 0x204e04;
inline constexpr std::uintptr_t v_cl_angles = 0x204df8;
inline constexpr std::uintptr_t v_sim_org = 0x204dec;
inline constexpr std::uintptr_t ev_punchangle = 0x204dcc;
inline constexpr std::uintptr_t g_PlayerExtraInfo = 0x1f9420;
inline constexpr std::uintptr_t g_PlayerInfoList = 0x1fb1a0;
inline constexpr std::uintptr_t g_lastFOV = 0x1f9400;
inline constexpr std::uintptr_t pmove = 0x228d64;
}
namespace linux_engine {
inline constexpr std::uintptr_t cl = 0xc2fa80;
inline constexpr std::uintptr_t cls = 0xde3c40;
inline constexpr std::uintptr_t cl_entities = 0x35f684;
inline constexpr std::uintptr_t r_refdef = 0xf7d8a0;
inline constexpr std::uintptr_t r_origin = 0xf7d760;
inline constexpr std::uintptr_t vpn = 0xf7d76c;
inline constexpr std::uintptr_t vright = 0xf7d784;
inline constexpr std::uintptr_t vup = 0xf7d980;
inline constexpr std::uintptr_t gWorldToScreen = 0xf7d9a0;
inline constexpr std::uintptr_t hudGetLocalPlayer = 0x156d60;
inline constexpr std::uintptr_t hudGetEntityByIndex = 0x156db0;
inline constexpr std::uintptr_t hudGetViewAngles = 0x156c70;
inline constexpr std::uintptr_t tri_ScreenTransform = 0x1b0480;
}
namespace engine_api32 {
inline constexpr std::size_t pfnGetScreenInfo = 0x30;
inline constexpr std::size_t GetViewAngles = 0x88;
inline constexpr std::size_t GetMaxClients = 0x90;
inline constexpr std::size_t GetLocalPlayer = 0xcc;
inline constexpr std::size_t GetViewModel = 0xd0;
inline constexpr std::size_t GetEntityByIndex = 0xd4;
inline constexpr std::size_t GetClientTime = 0xd8;
inline constexpr std::size_t pTriAPI = 0x148;
inline constexpr std::size_t pEventAPI = 0x150;
}

namespace ref_params_s_layout {
inline constexpr std::size_t size = 232;
inline constexpr std::size_t vieworg = 0x0;
inline constexpr std::size_t viewangles = 0xc;
inline constexpr std::size_t forward = 0x18;
inline constexpr std::size_t right = 0x24;
inline constexpr std::size_t up = 0x30;
inline constexpr std::size_t frametime = 0x3c;
inline constexpr std::size_t time = 0x40;
inline constexpr std::size_t intermission = 0x44;
inline constexpr std::size_t paused = 0x48;
inline constexpr std::size_t spectator = 0x4c;
inline constexpr std::size_t onground = 0x50;
inline constexpr std::size_t waterlevel = 0x54;
inline constexpr std::size_t simvel = 0x58;
inline constexpr std::size_t simorg = 0x64;
inline constexpr std::size_t viewheight = 0x70;
inline constexpr std::size_t idealpitch = 0x7c;
inline constexpr std::size_t cl_viewangles = 0x80;
inline constexpr std::size_t health = 0x8c;
inline constexpr std::size_t crosshairangle = 0x90;
inline constexpr std::size_t viewsize = 0x9c;
inline constexpr std::size_t punchangle = 0xa0;
inline constexpr std::size_t maxclients = 0xac;
inline constexpr std::size_t viewentity = 0xb0;
inline constexpr std::size_t playernum = 0xb4;
inline constexpr std::size_t max_entities = 0xb8;
inline constexpr std::size_t demoplayback = 0xbc;
inline constexpr std::size_t hardware = 0xc0;
inline constexpr std::size_t smoothing = 0xc4;
inline constexpr std::size_t cmd = 0xc8;
inline constexpr std::size_t movevars = 0xcc;
inline constexpr std::size_t viewport = 0xd0;
inline constexpr std::size_t nextView = 0xe0;
inline constexpr std::size_t onlyClientDraw = 0xe4;
}

namespace cl_entity_s_layout {
inline constexpr std::size_t size = 3000;
inline constexpr std::size_t index = 0x0;
inline constexpr std::size_t player = 0x4;
inline constexpr std::size_t baseline = 0x8;
inline constexpr std::size_t prevstate = 0x15c;
inline constexpr std::size_t curstate = 0x2b0;
inline constexpr std::size_t current_position = 0x404;
inline constexpr std::size_t ph = 0x408;
inline constexpr std::size_t mouth = 0xb08;
inline constexpr std::size_t latched = 0xb10;
inline constexpr std::size_t lastmove = 0xb44;
inline constexpr std::size_t origin = 0xb48;
inline constexpr std::size_t angles = 0xb54;
inline constexpr std::size_t attachment = 0xb60;
inline constexpr std::size_t trivial_accept = 0xb90;
inline constexpr std::size_t model = 0xb94;
inline constexpr std::size_t efrag = 0xb98;
inline constexpr std::size_t topnode = 0xb9c;
inline constexpr std::size_t syncbase = 0xba0;
inline constexpr std::size_t visframe = 0xba4;
inline constexpr std::size_t cvFloorColor = 0xba8;
}

namespace entity_state_s_layout {
inline constexpr std::size_t size = 340;
inline constexpr std::size_t entityType = 0x0;
inline constexpr std::size_t number = 0x4;
inline constexpr std::size_t msg_time = 0x8;
inline constexpr std::size_t messagenum = 0xc;
inline constexpr std::size_t origin = 0x10;
inline constexpr std::size_t angles = 0x1c;
inline constexpr std::size_t modelindex = 0x28;
inline constexpr std::size_t sequence = 0x2c;
inline constexpr std::size_t frame = 0x30;
inline constexpr std::size_t colormap = 0x34;
inline constexpr std::size_t skin = 0x38;
inline constexpr std::size_t solid = 0x3a;
inline constexpr std::size_t effects = 0x3c;
inline constexpr std::size_t scale = 0x40;
inline constexpr std::size_t eflags = 0x44;
inline constexpr std::size_t rendermode = 0x48;
inline constexpr std::size_t renderamt = 0x4c;
inline constexpr std::size_t rendercolor = 0x50;
inline constexpr std::size_t renderfx = 0x54;
inline constexpr std::size_t movetype = 0x58;
inline constexpr std::size_t animtime = 0x5c;
inline constexpr std::size_t framerate = 0x60;
inline constexpr std::size_t body = 0x64;
inline constexpr std::size_t controller = 0x68;
inline constexpr std::size_t blending = 0x6c;
inline constexpr std::size_t velocity = 0x70;
inline constexpr std::size_t mins = 0x7c;
inline constexpr std::size_t maxs = 0x88;
inline constexpr std::size_t aiment = 0x94;
inline constexpr std::size_t owner = 0x98;
inline constexpr std::size_t friction = 0x9c;
inline constexpr std::size_t gravity = 0xa0;
inline constexpr std::size_t team = 0xa4;
inline constexpr std::size_t playerclass = 0xa8;
inline constexpr std::size_t health = 0xac;
inline constexpr std::size_t spectator = 0xb0;
inline constexpr std::size_t weaponmodel = 0xb4;
inline constexpr std::size_t gaitsequence = 0xb8;
inline constexpr std::size_t basevelocity = 0xbc;
inline constexpr std::size_t usehull = 0xc8;
inline constexpr std::size_t oldbuttons = 0xcc;
inline constexpr std::size_t onground = 0xd0;
inline constexpr std::size_t iStepLeft = 0xd4;
inline constexpr std::size_t flFallVelocity = 0xd8;
inline constexpr std::size_t fov = 0xdc;
inline constexpr std::size_t weaponanim = 0xe0;
inline constexpr std::size_t startpos = 0xe4;
inline constexpr std::size_t endpos = 0xf0;
inline constexpr std::size_t impacttime = 0xfc;
inline constexpr std::size_t starttime = 0x100;
inline constexpr std::size_t iuser1 = 0x104;
inline constexpr std::size_t iuser2 = 0x108;
inline constexpr std::size_t iuser3 = 0x10c;
inline constexpr std::size_t iuser4 = 0x110;
inline constexpr std::size_t fuser1 = 0x114;
inline constexpr std::size_t fuser2 = 0x118;
inline constexpr std::size_t fuser3 = 0x11c;
inline constexpr std::size_t fuser4 = 0x120;
inline constexpr std::size_t vuser1 = 0x124;
inline constexpr std::size_t vuser2 = 0x130;
inline constexpr std::size_t vuser3 = 0x13c;
inline constexpr std::size_t vuser4 = 0x148;
}

namespace client_data_s_layout {
inline constexpr std::size_t size = 32;
inline constexpr std::size_t origin = 0x0;
inline constexpr std::size_t viewangles = 0xc;
inline constexpr std::size_t iWeaponBits = 0x18;
inline constexpr std::size_t fov = 0x1c;
}

namespace clientdata_s_layout {
inline constexpr std::size_t size = 476;
inline constexpr std::size_t origin = 0x0;
inline constexpr std::size_t velocity = 0xc;
inline constexpr std::size_t viewmodel = 0x18;
inline constexpr std::size_t punchangle = 0x1c;
inline constexpr std::size_t flags = 0x28;
inline constexpr std::size_t waterlevel = 0x2c;
inline constexpr std::size_t watertype = 0x30;
inline constexpr std::size_t view_ofs = 0x34;
inline constexpr std::size_t health = 0x40;
inline constexpr std::size_t bInDuck = 0x44;
inline constexpr std::size_t weapons = 0x48;
inline constexpr std::size_t flTimeStepSound = 0x4c;
inline constexpr std::size_t flDuckTime = 0x50;
inline constexpr std::size_t flSwimTime = 0x54;
inline constexpr std::size_t waterjumptime = 0x58;
inline constexpr std::size_t maxspeed = 0x5c;
inline constexpr std::size_t fov = 0x60;
inline constexpr std::size_t weaponanim = 0x64;
inline constexpr std::size_t m_iId = 0x68;
inline constexpr std::size_t ammo_shells = 0x6c;
inline constexpr std::size_t ammo_nails = 0x70;
inline constexpr std::size_t ammo_cells = 0x74;
inline constexpr std::size_t ammo_rockets = 0x78;
inline constexpr std::size_t m_flNextAttack = 0x7c;
inline constexpr std::size_t tfstate = 0x80;
inline constexpr std::size_t pushmsec = 0x84;
inline constexpr std::size_t deadflag = 0x88;
inline constexpr std::size_t physinfo = 0x8c;
inline constexpr std::size_t iuser1 = 0x18c;
inline constexpr std::size_t iuser2 = 0x190;
inline constexpr std::size_t iuser3 = 0x194;
inline constexpr std::size_t iuser4 = 0x198;
inline constexpr std::size_t fuser1 = 0x19c;
inline constexpr std::size_t fuser2 = 0x1a0;
inline constexpr std::size_t fuser3 = 0x1a4;
inline constexpr std::size_t fuser4 = 0x1a8;
inline constexpr std::size_t vuser1 = 0x1ac;
inline constexpr std::size_t vuser2 = 0x1b8;
inline constexpr std::size_t vuser3 = 0x1c4;
inline constexpr std::size_t vuser4 = 0x1d0;
}

namespace usercmd_s_layout {
inline constexpr std::size_t size = 52;
inline constexpr std::size_t lerp_msec = 0x0;
inline constexpr std::size_t msec = 0x2;
inline constexpr std::size_t viewangles = 0x4;
inline constexpr std::size_t forwardmove = 0x10;
inline constexpr std::size_t sidemove = 0x14;
inline constexpr std::size_t upmove = 0x18;
inline constexpr std::size_t lightlevel = 0x1c;
inline constexpr std::size_t buttons = 0x1e;
inline constexpr std::size_t impulse = 0x20;
inline constexpr std::size_t weaponselect = 0x21;
inline constexpr std::size_t impact_index = 0x24;
inline constexpr std::size_t impact_position = 0x28;
}

namespace extra_player_info_t_layout {
inline constexpr std::size_t size = 116;
inline constexpr std::size_t frags = 0x0;
inline constexpr std::size_t deaths = 0x2;
inline constexpr std::size_t team_id = 0x4;
inline constexpr std::size_t has_c4 = 0x8;
inline constexpr std::size_t vip = 0xc;
inline constexpr std::size_t origin = 0x10;
inline constexpr std::size_t radarflash = 0x1c;
inline constexpr std::size_t radarflashon = 0x20;
inline constexpr std::size_t radarflashes = 0x24;
inline constexpr std::size_t playerclass = 0x28;
inline constexpr std::size_t teamnumber = 0x2a;
inline constexpr std::size_t teamname = 0x2c;
inline constexpr std::size_t dead = 0x3c;
inline constexpr std::size_t showhealth = 0x40;
inline constexpr std::size_t health = 0x44;
inline constexpr std::size_t location = 0x48;
inline constexpr std::size_t sb_health = 0x68;
inline constexpr std::size_t sb_account = 0x6c;
inline constexpr std::size_t has_defuse_kit = 0x70;
}
}
