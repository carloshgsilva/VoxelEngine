#version 450

const vec2 QUAD[]={
    vec2(-1.0,-1.0),
    vec2(1.0,-1.0),
    vec2(-1.0,1.0),
    vec2(-1.0,1.0),
    vec2(1.0,-1.0),
    vec2(1.0,1.0),
};

layout(location=0) out struct{
    vec2 UV;
} Out;

void main() {
    vec3 Pos = vec3(QUAD[gl_VertexIndex], 0);
    Out.UV = (Pos * 0.5 + 0.5).xy;
    Out.UV.y = 1.0 - Out.UV.y;
    gl_Position = vec4(Pos, 1.0);
}
