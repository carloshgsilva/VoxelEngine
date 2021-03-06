#version 450

#include "lib/Common.frag"

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _LightTextureRID;
    int _NormalTextureRID;
    int _MaterialTextureRID;
    int _DepthTextureRID;
    int _BlueNoiseTextureRID;
    int _SkyBoxTextureRID;
    int _ShadowVoxRID;
};

BINDING_VIEW_BUFFER()

#define IMPORT
#include "lib/PBR.frag"
#include "lib/Light.frag"


layout(location=0) in struct {
    vec2 UV;
    vec3 FarVec;
} In;

layout(location=0) out vec4 out_Color;

vec4 getNoise(int s) {
    vec2 res = textureSize(DEPTH_TEXTURE, 0);
    return texelFetch(BLUE_NOISE_TEXTURE, ivec2((In.UV+vec2(GOLDEN_RATIO*(mod(GetFrame()+s*5,64)), GOLDEN_RATIO*(mod(GetFrame()+s*7+1,64))))*res)%512, 0);
}

vec4 getNoise(){
    vec2 res = textureSize(DEPTH_TEXTURE, 0);
    return texelFetch(BLUE_NOISE_TEXTURE, ivec2((In.UV+vec2(GOLDEN_RATIO*(mod(GetFrame(),16)), GOLDEN_RATIO*(mod(GetFrame()+1,16))))*res)%512, 0);
}

vec2 worldToUV(vec3 pos){
    vec4 p = GetProjectionMatrix() * vec4(pos, 1.0);
    return (p.xy / (p.w)*vec2(1.0, -1.0) )*0.5 + 0.5;
}

vec3 cosineSampleHemisphere(vec2 rand) {
	float r = sqrt(rand.x);
	float theta = 6.283 * rand.y;
	float x = r * cos(theta);
	float y = r * sin(theta);
	return vec3(x, y, sqrt(max(0.0, 1.0 - rand.x)));
}

vec3 getSkyColor(vec3 e) {
    return texture(SKY_BOX_TEXTURE, e).rgb;
    //e.y = (max(e.y,0.0)*0.8+0.2)*0.8;
    //return vec3(pow(1.0-e.y,2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * 1.1;
}

//Everything is calculated in ViewSpace to avoid float precision issues
void main() {
    vec2 uv = In.UV;
    vec4 matl = texture(MATERIAL_TEXTURE, uv);
    float depth = texture(DEPTH_TEXTURE, uv).r;
    vec3 pos = In.FarVec*(depth*(1.0+1/FAR)); // ViewSpace
    vec3 normal = texture(NORMAL_TEXTURE, uv).xyz; // WorldSpace

    //PBR Material
    float roughness = matl.r;
    float metallic = matl.g;
    //float emit = matl.b;

    // Directional Light Calculation

    vec3 albedo = vec3(1.0,1.0,1.0);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 V = -normalize(pos); // ViewSpace
    vec3 N = (GetViewMatrix()*vec4(normal, 0.0)).xyz; // ViewSpace
    vec3 R = reflect(-V, N);// Reflection

    //Light Ambient
    vec3 ambient = vec3(0,0,0);
       
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness)*5.0+0.0;
     
    if(depth < 0.999){
        //TODO: use R to calculate if need reflection
        //Specular Occlusion
        //Specular
        vec3 wd = normalize((GetInverseViewMatrix()*vec4(R, 0.0)).xyz);
        vec3 wcp = (GetInverseViewMatrix()*vec4(pos, 1.0)).xyz*10.0;
        vec3 randomVec = cosineSampleHemisphere(getNoise().xy);
        randomVec.z *= sign(getNoise().z-0.5);
        wd = mix(wd, randomVec, roughness*0.1);
        wcp += normal*getNoise().w;
        wd *= 1.0+getNoise().w*0.5;


        #if 0
            vec3 hitPos;
            vec3 hitNormal;
            if(raycastShadowVolume(wcp+normal, wd, 256.0, hitPos, hitNormal)){
                
                vec3 pos = (GetViewMatrix()*vec4((hitPos)*0.1, 1)).xyz;
                vec2 uv = worldToUV(pos);
                ambient += texture(LIGHT_TEXTURE, uv).rgb;
            }else{
                ambient += getSkyColor(wd);
            }
        #else
            float t = raycastShadowVolumeSparse(wcp+normal, wd, 256.0);
            
            vec3 pos = (GetViewMatrix()*vec4((wcp + wd*t)*0.1, 1)).xyz;
            vec4 projPos = GetProjectionMatrix()*vec4(pos, 1);
            float linearDepth = ((projPos.w-NEAR)/(FAR-NEAR));
            vec2 uv = worldToUV(pos);
            float depth = texture(DEPTH_TEXTURE, uv).r;
            
            if(t == 256.0){
                ambient += getSkyColor(wd);
            }else{
                if(linearDepth > depth - 0.001){ // If point hits depth
                    if(linearDepth < depth + 0.001){ // If point not visible
                        ambient += texture(LIGHT_TEXTURE, uv).rgb;
                    }
                }else{
                    if(t == 256.0){
                        ambient += getSkyColor(wd);
                    }
                }
            }
            
        #endif
        
    }
    
    out_Color = vec4(ambient*F*(1.0-roughness), F);
}