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
    int _DepthTextureRID;
};

BINDING_VIEW_BUFFER()


layout(location=0) out struct{
    vec3 FarVec;
} Out;

vec3 ComputeFarVec(vec2 pos){
    vec4 v = vec4(pos, 1.0, 1.0);
    v = inverse(GetProjectionMatrix()*GetViewMatrix()) * v;
    return v.xyz / v.w - GetCameraPosition();
}

void main() {
    vec3 Pos = QUAD[gl_VertexIndex];
    
    Out.FarVec = ComputeFarVec(Pos.xy);

    gl_Position = vec4(Pos, 1.0);
}
