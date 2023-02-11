#include "Common.frag"

vec2 OFFSETS[4] = vec2[4](
    vec2(-1,0), vec2(1, 0), 
	vec2(0, -1), vec2(0, 1));

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _MotionTextureRID;
    int _LastColorTextureRID;
    int _ColorTextureRID;
};

#define IMPORT
#include "View.frag"

layout(location=0) in struct {
    vec2 UV;
} In;

layout(location=0) out vec4 out_Color;

void main() {
    const float UPSAMPLING_FACTOR = 1.0;

    vec2 res = textureSize(COLOR_TEXTURE, 0);

    vec2 motion = texelFetch(MOTION_TEXTURE, ivec2(In.UV*res/UPSAMPLING_FACTOR), 0).xy;
    vec4 col = texture(COLOR_TEXTURE, In.UV, 0);
    vec3 currentColor = col.rgb;
    
    vec2 iRes = 1.0/res;
    vec2 oldUV = In.UV+motion;

    if(clamp(oldUV, 0.001, 0.999) != oldUV){
        oldUV = In.UV;
        out_Color = vec4(currentColor,1);
        return;
    }
    vec4 lstColorRaw = texture(LAST_COLOR_TEXTURE, oldUV);
    vec3 lastColor = lstColorRaw.rgb;
    float lastClip = lstColorRaw.a;
    vec2 oldMotion = texelFetch(MOTION_TEXTURE, ivec2(oldUV*res/UPSAMPLING_FACTOR), 0).xy;

    //Neighbour clip
    vec3 ref0 = texelFetch(COLOR_TEXTURE, ivec2(In.UV*res)+ivec2(1, 1) , 0).xyz;
    vec3 ref1 = texelFetch(COLOR_TEXTURE, ivec2(In.UV*res)-ivec2(1, 1) , 0).xyz;
    vec3 ref2 = texelFetch(COLOR_TEXTURE, ivec2(In.UV*res)+ivec2(-1, 1), 0).xyz;
    vec3 ref3 = texelFetch(COLOR_TEXTURE, ivec2(In.UV*res)-ivec2(-1, 1), 0).xyz;

    vec3 nmin = min(ref0, min(ref1, min(ref2, ref3)));
    vec3 nmax = max(ref0, max(ref1, max(ref2, ref3)));
    
    vec3 old0 = texelFetch(LAST_COLOR_TEXTURE, ivec2(oldUV*res)+ivec2(1, 1) , 0).xyz;
    vec3 old1 = texelFetch(LAST_COLOR_TEXTURE, ivec2(oldUV*res)-ivec2(1, 1) , 0).xyz;
    vec3 old2 = texelFetch(LAST_COLOR_TEXTURE, ivec2(oldUV*res)+ivec2(-1, 1), 0).xyz;
    vec3 old3 = texelFetch(LAST_COLOR_TEXTURE, ivec2(oldUV*res)-ivec2(-1, 1), 0).xyz;

    vec3 omin = min(old0, min(old1, min(old2, old3)));
    vec3 omax = max(old0, max(old1, max(old2, old3)));

    //How much this pixel is the rendered one of the upsampling
    float factor = length(mod(In.UV*res-vec2(1,0), 2) - GetJitter()*2);
    
    //How much clamping
    float t = lastClip + length(motion-oldMotion)*100.0;
    t += 0.1*max(t, (nmin.r + nmin.g + nmin.b) / (omax.r + omax.g + omax.b + 0.1)-0.5)*(1.0-col.a);
	t += 0.1*max(t, (omin.r + omin.g + omin.b) / (nmax.r + nmax.g + nmax.b + 0.1)-0.5)*(1.0-col.a);
    t = clamp(t, 0.0, 1.0);

    //Sharpening filter
    currentColor = mix(currentColor, currentColor*2 - 1*(0.25*(ref0+ref1+ref2+ref3)), t);
    //lastColor = mix(lastColor, lastColor*2.0 - 0.25*(old0+old1+old2+old3), t);
    
    //Variable color clamping
    lastColor = mix(lastColor, clamp(lastColor, nmin, nmax), t);
    
    out_Color = vec4(mix(currentColor, lastColor, 0.95), t*0.7);
}