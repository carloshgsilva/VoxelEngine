#version 450

#include "lib/Common.frag"

layout(push_constant) uniform uPushConstant{
    int _LightTextureRID;
};

layout(location=0) in struct {
    vec2 UV;
} In;

layout(location=0) out vec4 out_Color;

void main() {
    vec3 light = vec3(texture(LIGHT_TEXTURE, In.UV).rgb);
    float brightness = dot(light, vec3(0.2126, 0.7152, 0.0722));

    out_Color = vec4(light*step(0.3, brightness)*0.1, 1.0);
}