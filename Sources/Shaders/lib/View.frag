#include "Common.frag"

#ifndef VIEW_H
#define VIEW_H

#include "Math.frag"

#ifndef IMPORT
const int _ViewBufferRID = 0;
#endif

BINDING_BUFFER_R(ViewBuffer,
    mat4 LastViewMatrix;
	mat4 ViewMatrix;
	mat4 InverseViewMatrix;
	mat4 ProjectionMatrix;
	mat4 InverseProjectionMatrix;
    vec2 Res;
    vec2 iRes;
    vec3 CameraPosition;
    vec2 Jitter;
	int Frame;
    int _ColorTextureRID;
    int _DepthTextureRID;
    int _PalleteColorRID;
    int _PalleteMaterialRID;
	int BlueNoiseRID;
    float Time;
)
#define VIEW ViewBuffer[_ViewBufferRID]

vec3 GetSkyColor(vec3 e) {
    float f = max(sin(M_PI*VIEW.Time/24.0), 0.03);
    e.y = (max(e.y,0.0)*0.8+0.2)*0.8;
    return vec3(pow(1.0-e.y,2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * f;
}
vec3 GetSunDir() {
    float t = VIEW.Time*M_PI/12.0;
    return normalize(vec3(0.9, -cos(t), sin(t)));
}
vec3 GetSunColor() {
    float f = sin(M_PI*VIEW.Time/24.0);
    return vec3(0.9,0.9,0.8)*1.0;
}

mat4 GetLastViewMatrix(){ return VIEW.LastViewMatrix; }
mat4 GetViewMatrix(){ return VIEW.ViewMatrix; }
mat4 GetInverseViewMatrix(){ return VIEW.InverseViewMatrix; }
mat4 GetProjectionMatrix(){ return VIEW.ProjectionMatrix; }
mat4 GetInverseProjectionMatrix(){ return VIEW.InverseProjectionMatrix; }
vec2 GetRes(){ return VIEW.Res; }
vec2 GetiRes(){ return VIEW.iRes; }
vec3 GetCameraPosition(){ return VIEW.CameraPosition; }
vec2 GetJitter(){ return VIEW.Jitter; }
int GetFrame(){ return VIEW.Frame; }

vec3 WorldToView(vec3 p) {
    return (GetViewMatrix()*vec4(p, 1)).xyz;
}
vec2 ViewToUV(vec3 pos) {
    vec4 p = GetProjectionMatrix() * vec4(pos, 1.0);
    return (p.xy / (p.w)*vec2(1.0, -1.0))*0.5 + 0.5;
}
vec3 ViewToWorld(vec3 p) {
    return (GetInverseViewMatrix()*vec4(p, 1)).xyz;
}

vec2 WorldToUVLastView(vec3 world) {
    vec3 view = (VIEW.LastViewMatrix*vec4(world, 1)).xyz;
    vec4 p = GetProjectionMatrix() * vec4(view, 1.0);
    return (p.xy / (p.w)*vec2(1.0, -1.0))*0.5 + 0.5;
}
vec3 UVToWorldPreviousView(vec2 uv, float depth) {
    vec4 v = GetInverseProjectionMatrix() * vec4((uv-0.5)*vec2(2,-2), 1, 1);
    vec3 farVec = v.xyz / v.w;
    vec3 view = farVec*(depth*(1.0+1.0/FAR)+NEAR/FAR);
    return ViewToWorld(view);
}

vec3 UVDepthToView(vec2 uv, float depth) {
    vec4 v = GetInverseProjectionMatrix() * vec4((uv-0.5)*vec2(2,-2), 1, 1);
    vec3 farVec = v.xyz / v.w;
    return farVec*(depth*(1.0+1.0/FAR)+NEAR/FAR);
}
vec3 UVToRayDir(vec2 uv) {
    vec3 viewSpace = (GetInverseProjectionMatrix() * vec4((uv-0.5)*vec2(2, -2), 1, 1)).xyz;
    return (GetInverseViewMatrix() * vec4(viewSpace, 0)).xyz;
}
vec3 PrevousView_UVDepthToPos(vec2 uv, float depth) {
    vec3 viewSpace = (GetInverseProjectionMatrix() * vec4((uv-0.5)*vec2(2, -2), 1, 1)).xyz*depth;
    return (inverse(VIEW.LastViewMatrix) * vec4(viewSpace, 1)).xyz;
}


vec3 UVDepthToWorld(vec2 uv, float depth) {
    return GetCameraPosition() + UVToRayDir(uv)*depth;
}

#endif