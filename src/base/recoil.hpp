#pragma once
#include <algorithm>
namespace trace {


struct BurstShots {
 int previous_event{},weapon{},completed{};
 bool sampled{},was_firing{};
 void observe(int event,int weapon_id,bool firing) {
     event=std::max(0,event);
     if(!sampled||weapon_id!=weapon) {
         previous_event=event;weapon=weapon_id;completed=0;was_firing=firing;sampled=true;return;
     }
     int fired=event>=previous_event?event-previous_event:
         (previous_event>=500?std::max(0,600-previous_event+event):0);
     if(!firing)completed=0;
     else {if(!was_firing)completed=0;completed+=std::clamp(fired,0,32);}
     previous_event=event;was_firing=firing;
 }
 bool active_from(int start) const {return was_firing&&completed+1>=std::clamp(start,1,25);}
};
}
