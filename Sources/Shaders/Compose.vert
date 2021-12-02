#version 450

#include "lib/Common.frag"

const vec3 QUAD[]={
    vec3(-1.0,-1.0,0.0),
    vec3(1.0,-1.0,0.0),
    vec3(-1.0,1.0,0.0),
    vec3(-1.0,1.0,0.0),
    vec3(1.0,-1.0,0.0),
    vec3(1.0,1.0,0.0),
};


layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _ColorTextureRID;
    int _DepthTextureRID;
    int _NormalTextureRID;
    int _LightTextureRID;
    int _ReflectionTextureRID;
    int _Bloom0TextureRID;
    int _Bloom1TextureRID;
    int _Bloom2TextureRID;
    int _Bloom3TextureRID;
    int _Bloom4TextureRID;
};

BINDING_VIEW_BUFFER()

layout(location=0) out struct{
    vec2 UV;
    vec3 FarVec;
} Out;

vec3 ComputeFarVec(vec2 pos){
    vec4 v = vec4(pos, 1.0, 1.0);
    v = inverse(GetProjectionMatrix()*GetViewMatrix()) * v;
    return v.xyz / v.w - GetCameraPosition();
}

void main() {
    vec3 Pos = QUAD[gl_VertexIndex];
    Out.UV = (Pos * 0.5 + 0.5).xy;
    Out.UV.y = 1.0 - Out.UV.y;

    Out.FarVec = ComputeFarVec(Pos.xy);

    gl_Position = vec4(Pos, 1.0);
}
