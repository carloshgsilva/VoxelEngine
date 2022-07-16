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
    int _NormalTextureRID;
    int _MaterialTextureRID;
    int _DepthTextureRID;
    int _BlueNoiseTextureRID;
    int _ShadowVoxRID;
    int _SkyBoxTextureRID;
};

BINDING_VIEW_BUFFER()

layout(location=0) out struct{
    vec2 UV;
    vec3 FarVec;
} Out;

vec3 ComputeFarVec(vec2 pos){
    vec4 v = vec4(pos, 1.0, 1.0);
    v = GetInverseProjectionMatrix() * v;
    return v.xyz / v.w;// - CameraPosition;
}

void main() {
    vec3 Pos = QUAD[gl_VertexIndex];
    Out.UV = (Pos * 0.5 + 0.5).xy;
    Out.UV.y = 1.0 - Out.UV.y;

    Out.FarVec = ComputeFarVec(Pos.xy);

    gl_Position = vec4(Pos, 1.0);
}
