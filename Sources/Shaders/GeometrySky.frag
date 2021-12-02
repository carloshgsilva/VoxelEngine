#version 450

#include "lib/Common.frag"


layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _DepthTextureRID;
};

BINDING_VIEW_BUFFER()

layout(location=0) in struct {
    vec3 FarVec;
} In;

layout(location=0) out vec4 out_Color;
layout(location=1) out vec4 out_Normal;
layout(location=2) out vec4 out_Material;
layout(location=3) out vec2 out_Motion;

void main() {
    vec4 currentUV = (GetProjectionMatrix()*GetViewMatrix()*vec4(In.FarVec*0.1, 1));
    vec4 lastUV = (GetProjectionMatrix()*GetLastViewMatrix()*vec4(In.FarVec*0.1, 1));

    out_Color = vec4(1,0,0,0);
    out_Material = vec4(0);
    out_Normal = vec4(0);
    out_Motion =  -vec2(0.5,-0.5)*((currentUV.xy/currentUV.w - lastUV.xy/lastUV.w));
    gl_FragDepth = 1.0;
}