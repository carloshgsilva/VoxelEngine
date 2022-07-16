#version 450

const vec3 QUAD[]={
    vec3(-1.0,-1.0,0.0),
    vec3(1.0,-1.0,0.0),
    vec3(-1.0,1.0,0.0),
    vec3(-1.0,1.0,0.0),
    vec3(1.0,-1.0,0.0),
    vec3(1.0,1.0,0.0),
};

layout(location=0) out struct{
    vec2 UV;
} Out;

void main() {
    vec3 Pos = QUAD[gl_VertexIndex];
    Out.UV = (Pos * 0.5 + 0.5).xy;
    Out.UV.y = 1.0 - Out.UV.y;
    gl_Position = vec4(Pos, 1.0);
}
