#version 450

#include "lib/Common.frag"

#define ENABLE_SHADOWS 1
#define ENABLE_OCCLUSION 1
#define HIGH_PRECISION_SHADOWS 0

const float OCCLUSION_STRENGTH = 3.0;
const float OCCLUSION_SIZE = 0.2;
const float OCCLUSION_TICKNESS = 0.2;
const float OCCLUSION_SIZE_BY_DISTANCE = 0.0;

const int SAMPLES = 4;
const vec3 SUN_DIR = normalize(vec3(0.3,0.4,0.5));
const vec3 SUN_COLOR = vec3(0.9,0.9,0.8)*0.5;
const float AMBIENT_LIGHT_FACTOR = 0.05;

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _ColorTextureRID;
    int _NormalTextureRID;
    int _MaterialTextureRID;
    int _DepthTextureRID;
    int _BlueNoiseTextureRID;
    int _ShadowVoxRID;
    int _SkyBoxTextureRID;
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

float screenspaceOcclusion(vec3 pos, vec3 dir, float dist){
    vec3 mid = pos+dir*dist;

    vec4 midProj = GetProjectionMatrix() * vec4(mid, 1.0);
    float sampleDepth = (midProj.w-NEAR)/(FAR-NEAR);


    vec2 uv = worldToUV(mid);
    float minDepth = texture(DEPTH_TEXTURE, uv).r;
    float maxDepth = minDepth+OCCLUSION_TICKNESS/FAR;
    
    if(clamp(uv, 0, 1) != uv){
        return 0.0;
    }

    //return step(minDepth,sampleDepth)*100.0;
    if(minDepth < sampleDepth && sampleDepth < maxDepth){
        return smoothstep(maxDepth, minDepth, sampleDepth)*dist;
    }
    return 0.0;
}

vec3 cosineSampleHemisphere(vec2 rand) {
	float r = sqrt(rand.x);
	float theta = 6.283 * rand.y;
	float x = r * cos(theta);
	float y = r * sin(theta);
	return vec3(x, y, sqrt(max(0.0, 1.0 - rand.x)));
}

float calculateOcclusion(vec3 normal){
    vec3 tangent = normalize(abs(normal.z) > 0.5 ? vec3(0.0, -normal.z, normal.y) : vec3(-normal.y, normal.x, 0.0));
    vec3 bitangent = normalize(cross(normal, tangent));

    float depth = texture(DEPTH_TEXTURE, In.UV).r;
    vec3 pos = In.FarVec*(depth*(1.0+1.0/(FAR))+NEAR/FAR);
    
    float occlusion = 0.0;
    float sizeMultiplier = OCCLUSION_SIZE*(1.0+depth*OCCLUSION_SIZE_BY_DISTANCE);
    for(int i = 0; i < SAMPLES; i++){
        vec4 randNoise = getNoise(i);
        vec3 randomVec = cosineSampleHemisphere(randNoise.xy);
        
        vec3 dir = tangent*randomVec.x + bitangent*randomVec.y + normal*randomVec.z;
        occlusion += screenspaceOcclusion(pos, normalize(dir)*sizeMultiplier, randNoise.b);
    }
    occlusion /= float(SAMPLES);
    occlusion *= OCCLUSION_STRENGTH;

    return clamp(1.0-occlusion, 0.0, 1.0);
}

vec3 calculateAmbientIrradiance(vec3 pos, vec3 normal) {
    vec3 tangent = abs(normal.z) > 0.5 ? vec3(0.0, -normal.z, normal.y) : vec3(-normal.y, normal.x, 0.0);
    vec3 bitangent = cross(normal, tangent);

    float occlusion = 0.0;
    
    vec4 randNoise = getNoise();
    vec3 randomVec = cosineSampleHemisphere(randNoise.xy);
    vec3 dir = tangent*randomVec.x + bitangent*randomVec.y + normal*randomVec.z;
    
    float d = raycastShadowVolumeSuperSparse(pos, dir,128.0)/128.0;
    occlusion += 1.0/(d);
    //return vec3((1.0-occlusion*0.3)*AMBIENT_LIGHT_FACTOR);
    //return (d==1)?texture(SKY_BOX_TEXTURE, dir).rgb*0.5:vec3(0.0);
    return vec3(d*d)*AMBIENT_LIGHT_FACTOR;
}

vec3 getSkyColor(vec3 e) {
    e.y = (max(e.y,0.0)*0.8+0.2)*0.8;
    return vec3(pow(1.0-e.y,2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * 1.1;
}

//Everything is calculated in ViewSpace to avoid float precision issues
void main() {
    vec2 uv = In.UV;
    float depth = texture(DEPTH_TEXTURE, uv).r;

    if(depth < 0.999){
        vec3 albedo = texture(COLOR_TEXTURE, uv).rgb;
        vec4 matl = texture(MATERIAL_TEXTURE, uv);
        vec3 pos = In.FarVec*(depth*(1.0+1/FAR)); // ViewSpace
        vec3 normal = texture(NORMAL_TEXTURE, uv).xyz; // WorldSpace

        float shadow = 1.0;
        vec3 ambientIrradiance = vec3(0.0);

        //ShadowVox Shadow Tracing
        {
            vec3 wd = SUN_DIR;
            vec3 wcp = (GetInverseViewMatrix()*vec4(pos, 1.0)).xyz*10.0;
            vec3 randomVec = cosineSampleHemisphere(getNoise().xy)*0.1;
            randomVec.z *= sign(getNoise().z-0.5);
            wd = mix(wd, randomVec, 0.5);
            wd = normalize(wd);
            wcp += wd * (getNoise().w*1.0);
            wcp += randomVec*2.5;

            float bias = smoothstep(0.0,0.2, depth)*50.0+1.5;
            #if ENABLE_SHADOWS
                #if HIGH_PRECISION_SHADOWS
                    vec3 hitPos;
                    vec3 hitNormal;
                    if(raycastShadowVolume(wcp+normal, wd, 100.0, hitPos, hitNormal)){
                        shadow = 0.0;
                    }
                #else
                    if(raycastShadowVolumeSparse(wcp+normal*bias, wd, 128.0) != 128.0){
                        shadow = 0.0;
                    }
                #endif
            #endif
            #if ENABLE_OCCLUSION
                ambientIrradiance = calculateAmbientIrradiance(wcp+normal*bias, normal);
            #endif
        }
    

        vec3 lightPos = (GetViewMatrix()*vec4(0,10,0, 1.0)).xyz;

        vec3 sunDir = (GetViewMatrix()*vec4(SUN_DIR, 0.0)).xyz;

        //PBR Material
        float roughness = matl.r;
        float metallic = matl.g;
        float emit = matl.b;

        // Directional Light Calculation
        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        vec3 V = -normalize(pos); // ViewSpace
        vec3 N = (GetViewMatrix()*vec4(normal, 0.0)).xyz; // ViewSpace
        vec3 L = sunDir;//normalize(lightPos-pos);
        vec3 R = reflect(-V, N);// Reflection

        float distance = length(lightPos - pos);
        float attenuation = 1.0;// / (distance * distance);

        vec3 radiance = SUN_COLOR*attenuation*shadow;
        vec3 Lo = PBRDirectLight(radiance, albedo, V, N, L, roughness, metallic);

        //Light Ambient
        vec3 ambient = vec3(0,0,0);
        {
            vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            vec3 irradiance = getSkyColor(vec3(1,0,0));
            vec3 diffuse = irradiance*albedo;
            
            ambient += diffuse * (emit*10.0 + kD * ambientIrradiance * calculateOcclusion(N));
        }
        
        out_Color = vec4(ambient + Lo, 0.0);
        
    } 
    else {
        vec3 eyeDir = normalize((GetInverseViewMatrix()*vec4(In.FarVec, 0.0)).xyz);
        out_Color = texture(SKY_BOX_TEXTURE, eyeDir);
    }
    //out_Color = vec4(vec3(occlusion), 0.0);
}