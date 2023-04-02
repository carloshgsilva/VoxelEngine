#ifndef COMMON_H
#define COMMON_H

#include "evk.frag"
#include "Math.frag"

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

vec3 RandomColor(uint s) {
    return fract(vec3(0.3, 0.4, 0.5)*s*0.001);
}

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

uvec4 GBufferFetch(int rid, ivec2 pixel) {
    return texelFetch(USampler2D[rid], pixel, 0);
}
uvec4 GBufferPack(float depth, vec3 normal, float rough, float metal, float emissive, uint visibility) {
    return uvec4(
        floatBitsToUint(depth),
        PackNormal(normal),
        packUnorm4x8(vec4(rough, metal, emissive, 0)),
        visibility
    );
}
float GBufferGetDepth(uvec4 packed) {
    return uintBitsToFloat(packed.x);
}
vec3 GBufferGetNormal(uvec4 packed) {
    return UnpackNormal(packed.y);
}
vec3 GBufferGetMaterial(uvec4 packed) {
    return unpackUnorm4x8(packed.z).xyz;
}
uint GBufferGetVisibility(uvec4 packed) {
    return packed.w;
}

#endif