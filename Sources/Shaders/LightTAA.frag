#version 450

#include "lib/Common.frag"

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _BlueNoiseTextureRID;
    int _MotionTextureRID;
    int _LightTextureRID;
    int _ColorTextureRID;
    int _NormalTextureRID;
    int _MaterialTextureRID;
    int _DepthTextureRID;
    int _LastLightTextureRID;
};

BINDING_VIEW_BUFFER()

layout(location=0) in struct {
    vec2 UV;
} In;

layout(location=0) out vec4 out_Color;

vec4 getNoise() {
    vec2 res = textureSize(DEPTH_TEXTURE, 0);
    return texelFetch(BLUE_NOISE_TEXTURE, ivec2((In.UV+vec2(GOLDEN_RATIO*(mod(GetFrame(),16)), GOLDEN_RATIO*(mod(GetFrame()+1,16))))*res)%512, 0);
}

float luminance(vec3 rgb)
{
    // Algorithm from Chapter 10 of Graphics Shaders.
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    return dot(rgb, W);
}

void main() {
    vec2 iRes = 1.0/textureSize(LIGHT_TEXTURE, 0);

    vec2 motion = texture(MOTION_TEXTURE, In.UV).xy;
    vec2 oldUV = In.UV+motion;
    vec4 currentLight = texture(LIGHT_TEXTURE, In.UV);
    vec3 currentColor = texture(COLOR_TEXTURE, In.UV).xyz;
    vec4 currentMaterial = texture(MATERIAL_TEXTURE, In.UV);
    vec3 currentNormal = texture(NORMAL_TEXTURE, In.UV).rgb;
    float currentDepth = texture(DEPTH_TEXTURE, In.UV).r;

    if(currentDepth == 1.0){
        out_Color = currentLight;
        return;
    }
    vec4 lastLight = texture(LAST_LIGHT_TEXTURE, oldUV);
    float lastVariance = lastLight.a;

    //Out of bounds (use only current frame)
    if(clamp(oldUV, 0.0, 1.0) != oldUV){
        float count = 0.0;
        vec3 neighColors = vec3(0.0, 0.0, 0.0);
        for (int x = -9; x <= 9; ++x) {
            for (int y = -9; y <= 9; ++y) {
                vec2 offset = vec2(float(x), float(y)) * iRes;
                vec2 uv = clamp(In.UV+offset, 0.001, 0.999);
                
                vec2 neighMotion = texture(MOTION_TEXTURE, uv).xy;
                vec3 neighColor = texture(COLOR_TEXTURE, uv).xyz;
                vec3 neighNormal = texture(NORMAL_TEXTURE, uv).rgb;
                vec4 neighMaterial = texture(MATERIAL_TEXTURE, uv);
                float neighDepth = texture(DEPTH_TEXTURE, uv).r;
                
                float factor = max(dot(currentNormal, neighNormal), 0);
                factor *= step(0.8, 1.0-length(currentMaterial-neighMaterial));
                factor *= 1.0-clamp(length(currentDepth-neighDepth)*FAR, 0, 1);
                factor *= 1.0-clamp(length(currentColor-neighColor), 0, 1);
                
                if(length(motion-neighMotion) > 0.1)factor = 0.0;
                
                vec3 neighLight = texture(LIGHT_TEXTURE, uv).rgb;
                neighColors += factor*neighLight;
                count += factor;
            }
        }
        out_Color = vec4(neighColors/count, 1);
        return;
    }
    

#if 1
    vec3 nmin = vec3(10000.0);
    vec3 nmax = vec3(0.0);
    vec3 neighColors = vec3(0.0);
    float diffSum = 1.0;
    float count = 0.0;
    float size = 12;
    float radius = 1.0;
    for(float angle = getNoise().x*3.1415*GOLDEN_RATIO; radius <= size; angle += 2.39) {
        radius += 1.0;

        vec2 offset = vec2(cos(angle), sin(angle)) * iRes * (radius*(lastVariance+1.0)) * clamp(0.1, 0.5, 1.0/currentDepth);
        vec2 uv = clamp(In.UV+offset, 0.001, 0.999);
        
        vec2 neighMotion = texture(MOTION_TEXTURE, uv).xy;
        vec3 neighColor = texture(COLOR_TEXTURE, uv).xyz;
        vec3 neighNormal = texture(NORMAL_TEXTURE, uv).rgb;
        vec4 neighMaterial = texture(MATERIAL_TEXTURE, uv);
        float neighDepth = texture(DEPTH_TEXTURE,  uv).r;
        
        float factor = 1.212 - radius/size;
        factor *= step(0.8, 1.0-length(currentMaterial-neighMaterial));
        factor *= max(dot(currentNormal, neighNormal), 0.0);
        factor *= 1.0-clamp(length(currentDepth-neighDepth)*FAR, 0, 1);
        factor *= 1.0-clamp(length(currentColor-neighColor)*10000.0, 0, 1);
        if(length(motion-neighMotion) > 0.1)factor = 0.0;

        vec3 neighLight = texture(LIGHT_TEXTURE, uv).rgb;
        nmin = min(nmin, neighLight);
        nmax = max(nmax, neighLight);
        neighColors += factor*neighLight;
        count += factor;

        diffSum += length(neighLight - currentLight.xyz)*factor;
    }
    currentLight.xyz += neighColors;
    currentLight.xyz /= count+1.0;
    diffSum /= radius;

    
    //Neighbour Clip
    lastLight.xyz = clamp(lastLight.xyz, nmin, nmax);
    //lastLight.xyz = clamp(lastLight.xyz, vec3(0), vec3(99999999));

    float variance = luminance(abs((currentLight.xyz)-(lastLight.xyz)));
    variance = clamp(variance*5.5, 0, 1);
    lastVariance += variance;
    lastVariance -= diffSum*0.08;
    lastVariance += length(motion)*20;
#else
    lastLight = clamp(lastLight, 0, 10);
#endif
    
    out_Color = vec4(mix(currentLight.xyz, lastLight.xyz, clamp(1.0-lastVariance, 0.3, 0.9)), clamp(lastVariance*0.7, 0, 1));
    //out_Color = vec4(currentLight.xyz, 0.0);
}