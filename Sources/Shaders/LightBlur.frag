#version 450

#include "lib/Common.frag"

layout(push_constant) uniform uPushConstant{
    int _LightTextureRID;
    int Horizontal;
};

layout(location=0) in struct {
    vec2 UV;
} In;

layout(location=0) out vec4 out_Color;

void main() {
    vec2 res = 1.0/textureSize(LIGHT_TEXTURE, 0);
	//res.x*2.0 as when doing horizontal we also half the resolution
	vec2 offset = (Horizontal == 1) ? vec2(res.x*2.0, 0.0) : vec2(0.0, res.y);
	vec4 c = vec4(0.0);
	c += texture(LIGHT_TEXTURE, In.UV - offset*3.0) * 0.07;
	c += texture(LIGHT_TEXTURE, In.UV - offset*2.0) * 0.13;
	c += texture(LIGHT_TEXTURE, In.UV - offset*1.0) * 0.19;
	c += texture(LIGHT_TEXTURE, In.UV) * 0.22;
	c += texture(LIGHT_TEXTURE, In.UV + offset*1.0) * 0.19;
	c += texture(LIGHT_TEXTURE, In.UV + offset*2.0) * 0.13;
	c += texture(LIGHT_TEXTURE, In.UV + offset*3.0) * 0.07;
    out_Color = c;
}