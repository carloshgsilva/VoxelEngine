#version 450

#include "lib/Common.frag"

layout(push_constant) uniform uPushConstant{
    int _ColorTextureRID;
};

layout(location=0) in struct {
    vec2 UV;
} In;

layout(location=0) out vec4 out_Color;

void main() {
    out_Color = vec4(texture(COLOR_TEXTURE, In.UV).rgb, 1.0);
}