#include "Common.frag"

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

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _VoxCmdsBufferRID;
    int _BlueNoiseRID;
    int VoxCmdIndex;
};

#define IMPORT
#include "View.frag"
BINDING_VOX_CMDS_BUFFER()


layout(location=0) out struct{
    vec3 localDirection;
    vec3 localCameraPos;
    mat4 MVPMatrix;
} Out;


void main() {
    VoxCmd cmd = GetVoxCmd(VoxCmdIndex);
    int _VolumeRID = cmd.VolumeRID;

    vec3 in_Pos = CUBE[gl_VertexIndex];
    vec3 volumeDimension = vec3(textureSize(VOLUME_TEXTURE, 0));
    Out.MVPMatrix = GetProjectionMatrix() * GetViewMatrix() * cmd.WorldMatrix;

    vec4 jitter = vec4(GetJitter()*GetiRes()*2.0, 0,0);
    
    vec3 localPos = in_Pos*volumeDimension;

    Out.localCameraPos = (inverse(cmd.WorldMatrix)*vec4(GetCameraPosition(), 1.0)).xyz*10.0;
    Out.localDirection = localPos - Out.localCameraPos;
    gl_Position = Out.MVPMatrix * vec4(localPos*0.1, 1);
    gl_Position += jitter*gl_Position.w;
}
