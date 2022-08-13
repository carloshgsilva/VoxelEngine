#include "Common.frag"

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
)

mat4 GetLastViewMatrix(){ return ViewBuffer[_ViewBufferRID].LastViewMatrix; }
mat4 GetViewMatrix(){ return ViewBuffer[_ViewBufferRID].ViewMatrix; }
mat4 GetInverseViewMatrix(){ return ViewBuffer[_ViewBufferRID].InverseViewMatrix; }
mat4 GetProjectionMatrix(){ return ViewBuffer[_ViewBufferRID].ProjectionMatrix; }
mat4 GetInverseProjectionMatrix(){ return ViewBuffer[_ViewBufferRID].InverseProjectionMatrix; }
vec2 GetRes(){ return ViewBuffer[_ViewBufferRID].Res; }
vec2 GetiRes(){ return ViewBuffer[_ViewBufferRID].iRes; }
vec3 GetCameraPosition(){ return ViewBuffer[_ViewBufferRID].CameraPosition; }
vec2 GetJitter(){ return ViewBuffer[_ViewBufferRID].Jitter; }
int GetFrame(){ return ViewBuffer[_ViewBufferRID].Frame; }

vec2 ViewToUV(vec3 pos) {
    vec4 p = GetProjectionMatrix() * vec4(pos, 1.0);
    return (p.xy / (p.w)*vec2(1.0, -1.0))*0.5 + 0.5;
}
vec3 UVDepthToView(vec2 uv, float depth) {
    vec4 v = GetInverseProjectionMatrix() * vec4((uv-0.5)*vec2(2,-2), 1, 1);
    vec3 farVec = v.xyz / v.w;
    return farVec*(depth*(1.0+1.0/FAR)+NEAR/FAR);
}
vec3 ViewToWorld(vec3 p) {
    return (GetInverseViewMatrix()*vec4(p, 1)).xyz;
}
vec3 UVToRayDir(vec2 uv) {
    vec3 viewSpace = (GetInverseProjectionMatrix() * vec4((uv-0.5)*vec2(2, -2), 1, 1)).xyz;
    return (GetInverseViewMatrix() * vec4(normalize(viewSpace), 0)).xyz;
}