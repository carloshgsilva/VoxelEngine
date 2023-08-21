#include "lib/Common.frag"

const vec2 RECT[6] = {
    vec2(0.0,  0.0),
    vec2(1.0,  0.0),
    vec2(0.0,  1.0),

    vec2(1.0,  0.0),
    vec2(0.0,  1.0),
    vec2(1.0,  1.0),
};

PUSH(
    vec2 ScreenSize;
    vec2 Position;
    vec2 Scale;
    float Time;
    int GlyphsBufferRID;
    int OffsetCoords;
    int NumCoords;
)

IN(0) vec2 Pos;
OUT(0) vec2 UV;

void main() {
    UV = Pos * Scale + Position;
    gl_Position = vec4(2.0 * (UV) / ScreenSize - 1.0, 0.0, 1.0);
}
