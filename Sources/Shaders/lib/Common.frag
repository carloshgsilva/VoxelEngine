#ifndef COMMON_H
#define COMMON_H

// requires: 
//  that push_constant have 'int _ViewBufferRID';

#extension GL_EXT_nonuniform_qualifier : require

#define GOLDEN_RATIO 2.118033988749895
#define MATH_PI 3.14159265359

const float NEAR = 0.1;
const float FAR = 4096.0;

struct PointLight {
    vec3 Position;
    float Range;
    vec3 Color;
    float Attenuation;
};

struct SpotLight {
    vec3 Position;
    float Range;
    vec3 Color;
    float Attenuation;
    vec3 Direction;
    float Angle;
    float AngleAttenuation;
};

struct VoxCmd {
    mat4 WorldMatrix;
    mat4 LastWorldMatrix;
    int VolumeRID;
    int PalleteIndex;
};

#define BINDING_BUFFER(name, code) layout(binding = 0) readonly buffer name##_t { code } name[];
layout(binding = 1) uniform sampler2D _BindingSampler2D[];
layout(binding = 1) uniform usampler3D _BindingUSampler3D[];
layout(binding = 1) uniform samplerCube _BindingSamplerCube[];

//ViewBuffer (requires: _ViewBufferRID)
#define BINDING_VIEW_BUFFER()     \
BINDING_BUFFER(ViewBuffer,        \
    mat4 LastViewMatrix;          \
	mat4 ViewMatrix;              \
	mat4 InverseViewMatrix;       \
	mat4 ProjectionMatrix;        \
	mat4 InverseProjectionMatrix; \
    vec2 Res;                     \
    vec2 iRes;                    \
    vec3 CameraPosition;          \
    vec2 Jitter;                  \
	int Frame;                    \
    int _ColorTextureRID;         \
    int _DepthTextureRID;         \
    int _PalleteColorRID;         \
    int _PalleteMaterialRID;      \
)                                 \
mat4 GetLastViewMatrix(){ return ViewBuffer[_ViewBufferRID].LastViewMatrix; }                   \
mat4 GetViewMatrix(){ return ViewBuffer[_ViewBufferRID].ViewMatrix; }                           \
mat4 GetInverseViewMatrix(){ return ViewBuffer[_ViewBufferRID].InverseViewMatrix; }             \
mat4 GetProjectionMatrix(){ return ViewBuffer[_ViewBufferRID].ProjectionMatrix; }               \
mat4 GetInverseProjectionMatrix(){ return ViewBuffer[_ViewBufferRID].InverseProjectionMatrix; } \
vec2 GetRes(){ return ViewBuffer[_ViewBufferRID].Res; }                                         \
vec2 GetiRes(){ return ViewBuffer[_ViewBufferRID].iRes; }                                       \
vec3 GetCameraPosition(){ return ViewBuffer[_ViewBufferRID].CameraPosition; }                   \
vec2 GetJitter(){ return ViewBuffer[_ViewBufferRID].Jitter; }                                    \
int GetFrame(){ return ViewBuffer[_ViewBufferRID].Frame; }                                      

//PointLights (requires: _PointLightsBufferRID)
#define BINDING_POINT_LIGHTS_BUFFER() \
BINDING_BUFFER(PointLightsBuffer,     \
    PointLight Lights[64];            \
)                                     \
PointLight GetPointLight(int index) { return PointLightsBuffer[_PointLightsBufferRID].Lights[index]; } \

//SpotLights (requires: _SpotLightsBufferRID)
#define BINDING_SPOT_LIGHTS_BUFFER() \
BINDING_BUFFER(SpotLightsBuffer,     \
    SpotLight Lights[64];            \
)                                     \
SpotLight GetSpotLight(int index) { return SpotLightsBuffer[_SpotLightsBufferRID].Lights[index]; } \

//VoxCmdsBuffer (requires: _VoxCmdsBufferRID)
#define BINDING_VOX_CMDS_BUFFER() \
BINDING_BUFFER(VoxCmdsBuffer,        \
    VoxCmd Cmds[4096];            \
)                                    \
VoxCmd GetVoxCmd(int index) { return VoxCmdsBuffer[_VoxCmdsBufferRID].Cmds[index]; } \



//View Textures
#define LAST_COLOR_TEXTURE _BindingSampler2D[_LastColorTextureRID]
#define COLOR_TEXTURE _BindingSampler2D[_ColorTextureRID]
#define DEPTH_TEXTURE _BindingSampler2D[_DepthTextureRID]
#define NORMAL_TEXTURE _BindingSampler2D[_NormalTextureRID]
#define MATERIAL_TEXTURE _BindingSampler2D[_MaterialTextureRID]
#define MOTION_TEXTURE _BindingSampler2D[_MotionTextureRID]
#define LAST_LIGHT_TEXTURE _BindingSampler2D[_LastLightTextureRID]
#define LIGHT_TEXTURE _BindingSampler2D[_LightTextureRID]
#define REFLECTION_TEXTURE _BindingSampler2D[_ReflectionTextureRID]
#define BLOOM_TEXTURE _BindingSampler2D[_BloomTextureRID]
#define OUTLINE_TEXTURE _BindingSampler2D[_OutlineTextureRID]
#define BLUE_NOISE_TEXTURE _BindingSampler2D[_BlueNoiseTextureRID]
#define PALLETE_COLOR_TEXTURE _BindingSampler2D[ViewBuffer[_ViewBufferRID]._PalleteColorRID]
#define PALLETE_MATERIAL_TEXTURE _BindingSampler2D[ViewBuffer[_ViewBufferRID]._PalleteMaterialRID]
#define VOLUME_TEXTURE _BindingUSampler3D[_VolumeRID]
#define SHADOW_VOX_TEXTURE _BindingUSampler3D[_ShadowVoxRID]
#define SKY_BOX_TEXTURE _BindingSamplerCube[_SkyBoxTextureRID]

#endif