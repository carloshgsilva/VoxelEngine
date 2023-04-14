#version 450

#include "lib/Common.frag"

const vec3 CUBE[]={
    vec3(0.0,1.0,1.0),
    vec3(0.0,0.0,1.0),
    vec3(0.0,0.0,0.0),
    vec3(1.0,1.0,1.0),
    vec3(1.0,0.0,1.0),
    vec3(0.0,0.0,1.0),
    vec3(1.0,1.0,0.0),
    vec3(1.0,0.0,0.0),
    vec3(1.0,0.0,1.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,0.0,0.0),
    vec3(1.0,0.0,0.0),
    vec3(0.0,0.0,1.0),
    vec3(1.0,0.0,1.0),
    vec3(1.0,0.0,0.0),
    vec3(1.0,1.0,1.0),
    vec3(0.0,1.0,1.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,1.0,1.0),
    vec3(0.0,0.0,0.0),
    vec3(0.0,1.0,1.0),
    vec3(1.0,1.0,1.0),
    vec3(0.0,0.0,1.0),
    vec3(1.0,1.0,1.0),
    vec3(1.0,1.0,0.0),
    vec3(1.0,0.0,1.0),
    vec3(1.0,1.0,0.0),
    vec3(0.0,1.0,0.0),
    vec3(1.0,0.0,0.0),
    vec3(0.0,0.0,0.0),
    vec3(0.0,0.0,1.0),
    vec3(1.0,0.0,0.0),
    vec3(1.0,1.0,0.0),
    vec3(1.0,1.0,1.0),
    vec3(0.0,1.0,0.0),
};


vec2 OFFSETS[8] = vec2[8](
    vec2(0.5,   0.333333),
    vec2(0.25,  0.666666),
    vec2(0.75,  0.111111),
    vec2(0.125, 0.444444),

    vec2(0.625, 0.777777),
    vec2(0.375, 0.222222),
    vec2(0.875, 0.555555),
    vec2(0.0625, 0.888888)
);


layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _VolumeRID;
    int _DepthTextureRID;
    mat4 u_WorldMatrix;
    vec3 u_OutlineColor;
};

#define IMPORT
#include "View.frag"

layout(location=0) out struct{
    vec3 localDirection;
    vec3 localCameraPos;
} Out;


void main() {
    vec3 in_Pos = CUBE[gl_VertexIndex];
    ivec3 u_VolumeDimension = textureSize(VOLUME_TEXTURE, 0);
    mat4 mvp = GetProjectionMatrix() * GetViewMatrix() * u_WorldMatrix;

    vec4 jitter = vec4(OFFSETS[GetFrame()%4]*GetiRes()*3.0,0,0);

    Out.localCameraPos = (inverse(u_WorldMatrix)*vec4(GetCameraPosition(), 1.0)).xyz*10.0;
    Out.localDirection = in_Pos*vec3(u_VolumeDimension) - Out.localCameraPos;
    gl_Position = mvp * vec4(in_Pos*0.1*vec3(u_VolumeDimension), 1);
    //gl_Position += jitter*gl_Position.w; //if it does not jit
}
