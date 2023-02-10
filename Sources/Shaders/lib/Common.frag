#ifndef COMMON_H
#define COMMON_H

#include "evk.frag"

// requires: 
//  that push_constant have 'int _ViewBufferRID';

#extension GL_EXT_nonuniform_qualifier : require

#define GOLDEN_RATIO 2.118033988749895
#define GOLDEN_ANGLE 2.39996322972865332
#define MATH_PI 3.14159265359

#define INF 100000.0
#define EPS 0.0001

const float NEAR = 0.1;
const float FAR = 4096.0;

float Luminance(vec3 c) {
    return dot(c, vec3(0.2125, 0.7154, 0.0721));
}

uint PackVisiblity(uint palleteId, uint voxel) {
    return (palleteId << 16) | (voxel);
}
void UnpackVisibility(uint packed, out uint palleteId, out uint voxel) {
    palleteId = (packed >> 16) & 0xFFFFu;
    voxel = (packed >> 0) & 0xFFFFu;
}

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
BINDING_BUFFER_R(VoxCmdsBuffer,        \
    VoxCmd Cmds[4096];            \
)                                    \
VoxCmd GetVoxCmd(int index) { return VoxCmdsBuffer[_VoxCmdsBufferRID].Cmds[index]; } \

//View Textures
#define LAST_COLOR_TEXTURE Sampler2D[_LastColorTextureRID]
#define COLOR_TEXTURE Sampler2D[_ColorTextureRID]
#define DEPTH_TEXTURE Sampler2D[_DepthTextureRID]
#define NORMAL_TEXTURE Sampler2D[_NormalTextureRID]
#define MATERIAL_TEXTURE Sampler2D[_MaterialTextureRID]
#define MOTION_TEXTURE Sampler2D[_MotionTextureRID]
#define LAST_LIGHT_TEXTURE Sampler2D[_LastLightTextureRID]
#define LIGHT_TEXTURE Sampler2D[_LightTextureRID]
#define REFLECTION_TEXTURE Sampler2D[_ReflectionTextureRID]
#define BLOOM_TEXTURE Sampler2D[_BloomTextureRID]
#define OUTLINE_TEXTURE Sampler2D[_OutlineTextureRID]
#define BLUE_NOISE_TEXTURE Sampler2D[_BlueNoiseTextureRID]
#define PALLETE_COLOR_TEXTURE Sampler2D[ViewBuffer[_ViewBufferRID]._PalleteColorRID]
#define PALLETE_MATERIAL_TEXTURE Sampler2D[ViewBuffer[_ViewBufferRID]._PalleteMaterialRID]
#define VOLUME_TEXTURE USampler3D[_VolumeRID]
#define SHADOW_VOX_TEXTURE USampler3D[_ShadowVoxRID]
#define SKY_BOX_TEXTURE SamplerCube[_SkyBoxTextureRID]

#endif