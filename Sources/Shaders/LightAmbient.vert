#include "evk.frag"

const vec3 QUAD[]={
    vec3(-1.0,-1.0,0.0),
    vec3(1.0,-1.0,0.0),
    vec3(-1.0,1.0,0.0),
    vec3(-1.0,1.0,0.0),
    vec3(1.0,-1.0,0.0),
    vec3(1.0,1.0,0.0),
};

OUT(0) vec2 UV;

void main() {
    vec3 Pos = QUAD[gl_VertexIndex];
    UV = (Pos * 0.5 + 0.5).xy;
    UV.y = 1.0 - UV.y;

    gl_Position = vec4(Pos, 1.0);
}
