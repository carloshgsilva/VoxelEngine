#version 450

#include "lib/Common.frag"

layout(push_constant) uniform uPushConstant{
    int _ColorTextureRID;
    int _OutlineTextureRID;
};

layout(location=0) in struct {
    vec2 UV;
} In;

layout(location=0) out vec4 out_Color;

void main() {
    vec2 res = (textureSize(COLOR_TEXTURE, 0));
    vec2 iRes = 1.0/res;

    vec3 olc = texelFetch(OUTLINE_TEXTURE, ivec2(In.UV*res), 0).rgb;
    vec3 oll = texelFetch(OUTLINE_TEXTURE, ivec2(In.UV*res)-ivec2(2, 0), 0).rgb;
    vec3 olr = texelFetch(OUTLINE_TEXTURE, ivec2(In.UV*res)+ivec2(2, 0), 0).rgb;
    vec3 olt = texelFetch(OUTLINE_TEXTURE, ivec2(In.UV*res)-ivec2(0, 2), 0).rgb;
    vec3 olb = texelFetch(OUTLINE_TEXTURE, ivec2(In.UV*res)+ivec2(0, 2), 0).rgb;

    vec3 overlay = vec3(0);

    vec2 screenCoords = floor(In.UV/iRes);
    if(texelFetch(OUTLINE_TEXTURE, ivec2(In.UV*res), 0).a < 0.5 && mod(screenCoords.x+screenCoords.y, 4).x==0){
        overlay = olc*0.3;
    }
    vec3 neigh = max(oll, max(olr, max(olt, olb)));
    vec3 outline = neigh != olc ? neigh : vec3(0);
    overlay += outline;

    out_Color = vec4(vec3(texture(COLOR_TEXTURE, In.UV).rgb) + overlay, 1.0);
}