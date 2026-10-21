#include "base/game.hpp"
#include "app/gui.h"
#include "pages/aim_page.h"
#include <cstdio>
#include <cstdlib>
void check(bool ok,const char* label){if(!ok){std::fprintf(stderr,"FAIL %s\n",label);std::exit(1);}}
int main(){using namespace trace;Game g;char api[540]{};g.api=api;g.camera_valid=true;g.local_index=1;g.local_team=1;g.count=1;
 g.players[0].team=2;g.players[0].index=2;g.players[0].visible=true;g.players[0].head_valid=true;g.players[0].head={1000,30,0};
 auto& cfg=gui::aim_settings();cfg.enabled=true;cfg.fov=5;cfg.smoothing=0;cfg.ignore_team=true;cfg.max_distance=250;
 UserCmd cmd{};g.aim(&cmd,.01,1,false,false);check(g.target==0,"requires attack by default");
 cmd.buttons=1;g.aim(&cmd,.01,1,false,false);check(g.target==2&&cmd.view.y>1,"visible enemy selected");
 cmd.view={};g.aim(&cmd,.01,1,true,false);check(g.target==0&&cmd.view.y==0,"menu suppresses aim");
 g.players[0].visible=false;g.aim(&cmd,.01,1,false,false);check(g.target==0,"occluded rejected");
 g.players[0].visible=true;g.players[0].head_valid=true;g.players[0].team=1;g.aim(&cmd,.01,1,false,false);check(g.target==0,"teammate rejected");
 g.players[0].team=2;g.local_team=0;g.aim(&cmd,.01,1,false,false);check(g.target==0,"unknown team rejected");
 g.local_team=1;g.players[0].head={0,1000,0};g.aim(&cmd,.01,1,false,false);check(g.target==0,"outside FOV rejected");
 g.players[0].head={1000,30,0};cfg.keybind=512;g.aim(&cmd,.01,1,false,false);check(g.target==0,"bound key must be held");
 g.aim(&cmd,.01,1,false,true);check(g.target==2,"held key activates aim");
 g.players[0].head_valid=false;g.aim(&cmd,.01,1,false,true);check(g.target==0,"missing head hitbox rejected");
 std::puts("PASS activation, visible selection, menu, occlusion, team, FOV, key gating");
}
