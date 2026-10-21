#include "base/sdk.hpp"
#include "base/recoil.hpp"
#include <cstdio>
#include <cstdlib>
void check(bool ok,const char* msg){if(!ok){std::fprintf(stderr,"FAIL %s\n",msg);std::exit(1);}}
int main(){using namespace trace;
 check(sizeof(void*)==4,"i386 ABI");check(std::abs(delta({0,-179,0},{0,179,0}).y-2)<.001f,"yaw wrap uses short path");
 auto a=angles({100,0,0});check(std::abs(a.x)<.001f&&std::abs(a.y)<.001f,"forward angle");
 a=angles({0,100,100});check(std::abs(a.x+45)<.001f&&std::abs(a.y-90)<.001f,"up/right angle");
 auto one=smooth({0,0,0},{20,40,0},50,.02f);auto two=smooth(smooth({0,0,0},{20,40,0},50,.01f),{20,40,0},50,.01f);
 check(std::abs(one.y-two.y)<.001f,"smoothing independent of command rate");
 float f=100,s=0;fix_movement(0,90,f,s);check(std::abs(f)<.001f&&std::abs(s-100)<.001f,"world movement preserved when turning");
 Matrix3x4 bone{{{0,-1,0,100},{1,0,0,200},{0,0,1,300}}};
 auto centre=transform_point({2,4,6},bone);
 check(centre.x==96&&centre.y==202&&centre.z==306,"animated hitbox uses bone rotation and translation");
 BurstShots shots;
 shots.observe(10,28,false);shots.observe(10,28,true);
 check(shots.active_from(1)&&!shots.active_from(2),"first-shot threshold");
 shots.observe(11,28,true);check(shots.active_from(2),"second-shot threshold uses actual fire event");
 shots.observe(33,28,true);check(!shots.active_from(25),"twenty-fifth shot is not early");
 shots.observe(34,28,true);check(shots.active_from(25),"twenty-fifth shot threshold");
 shots.observe(34,28,false);check(!shots.active_from(1)&&shots.completed==0,"release resets burst");
 shots.observe(35,29,true);check(!shots.active_from(2),"weapon change resets burst");
 BurstShots wrap;wrap.observe(599,28,false);wrap.observe(599,28,true);wrap.observe(1,28,true);
 check(wrap.completed==2,"local firing-event counter wrap");
 std::puts("PASS ABI, wrap, angles, smoothing, movement");
}
