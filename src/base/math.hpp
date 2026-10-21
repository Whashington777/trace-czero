#pragma once
#include <algorithm>
#include <cmath>
namespace trace {
struct Vec3 { float x{},y{},z{}; };
inline Vec3 operator+(Vec3 a,Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline Vec3 operator-(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline Vec3 operator*(Vec3 a,float b){return {a.x*b,a.y*b,a.z*b};}
inline bool finite(Vec3 a){return std::isfinite(a.x)&&std::isfinite(a.y)&&std::isfinite(a.z);}
struct Matrix3x4 { float row[3][4]{}; };
inline Vec3 transform_point(Vec3 p, const Matrix3x4& m) {
 return {m.row[0][0]*p.x+m.row[0][1]*p.y+m.row[0][2]*p.z+m.row[0][3],
         m.row[1][0]*p.x+m.row[1][1]*p.y+m.row[1][2]*p.z+m.row[1][3],
         m.row[2][0]*p.x+m.row[2][1]*p.y+m.row[2][2]*p.z+m.row[2][3]};
}
inline float length(Vec3 a){return std::sqrt(a.x*a.x+a.y*a.y+a.z*a.z);}
inline float yaw(float a){return std::remainder(a,360.f);}
inline Vec3 angles(Vec3 d){constexpr float rad=57.295779513f;return {-std::atan2(d.z,std::hypot(d.x,d.y))*rad,std::atan2(d.y,d.x)*rad,0};}
inline Vec3 delta(Vec3 target,Vec3 current){return {yaw(target.x-current.x),yaw(target.y-current.y),0};}
inline Vec3 smooth(Vec3 current,Vec3 target,float smoothing,float dt){
 auto d=delta(target,current);float tau=std::max(0.f,smoothing)*0.005f;
 float k=tau==0?1.f:1.f-std::exp(-std::clamp(dt,0.f,0.1f)/tau);
 return {std::clamp(current.x+d.x*k,-89.f,89.f),yaw(current.y+d.y*k),0};
}
inline void fix_movement(float old_yaw,float new_yaw,float& forward,float& side){
 float r=(new_yaw-old_yaw)*0.01745329252f;float f=forward,s=side;
 forward=std::cos(r)*f-std::sin(r)*s;side=std::sin(r)*f+std::cos(r)*s;
}
}
