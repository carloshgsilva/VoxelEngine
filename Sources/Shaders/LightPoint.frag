#version 450

#include "lib/Common.frag"

const float OCCLUSION_STRENGTH = 2.5;
const float OCCLUSION_SIZE = 0.1;
const float OCCLUSION_TICKNESS = 0.2;
const float OCCLUSION_SIZE_BY_DISTANCE = 0.0;

const int SAMPLES = 4;
const vec3 SUN_DIR = normalize(vec3(0.3,0.4,0.5));
const vec3 SUN_COLOR = vec3(0.9,0.9,0.8)*5.0;
const float AMBIENT_LIGHT_FACTOR = 0.5;

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _ColorTextureRID;
    int _NormalTextureRID;
    int _MaterialTextureRID;
    int _DepthTextureRID;
    int _BlueNoiseTextureRID;
    int _ShadowVoxRID;
    int _PointLightsBufferRID;
    int LightIndex;
};

BINDING_VIEW_BUFFER()
BINDING_POINT_LIGHTS_BUFFER()


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

vec3 clipToAABB(vec3 origin, vec3 dir, vec3 volumeSize) {
	if (clamp(origin, vec3(0.0), volumeSize) == origin)
		return origin;

	vec3 invDir = vec3(1.0) / dir;
	vec3 sgn = step(dir, vec3(0.0));
	vec3 t = (sgn*volumeSize - origin)*invDir;
    
	float tmin = max(max(t.x, t.y), t.z);

	return origin + dir*(tmin - 0.001);
}

vec3 getSkyColor(vec3 e) {
    e.y = (max(e.y,0.0)*0.8+0.2)*0.8;
    return vec3(pow(1.0-e.y,2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * 1.1;
}


//Everything is calculated in ViewSpace to avoid float precision issues
void main() {
    vec4 matl = texture(MATERIAL_TEXTURE, In.UV);
    vec3 albedo = texture(COLOR_TEXTURE, In.UV).rgb;
    float depth = texture(DEPTH_TEXTURE, In.UV).r;
    vec3 pos = In.FarVec*(depth*(1.0+1.0/FAR)); // ViewSpace
    vec3 normal = texture(NORMAL_TEXTURE, In.UV).xyz; // WorldSpace

    PointLight light = GetPointLight(LightIndex);


    vec3 worldPos = (GetInverseViewMatrix()*vec4(pos, 1.0)).xyz;
    vec3 lightPos = (GetViewMatrix()*vec4(light.Position, 1.0)).xyz;
    vec3 lightDir = light.Position - worldPos;
    float lightDistance = length(lightDir);

    if(lightDistance > light.Range){
        discard;
        return;
    }

    //ShadowVox Shadow Tracing
    float shadow = 1.0;
    {
        vec3 wd = lightDir;
        float hitDist = lightDistance*10.5;
        vec3 wcp = worldPos*10.0;
        vec3 randomVec = cosineSampleHemisphere(getNoise().xy)*0.1;
        randomVec.z *= sign(getNoise().z-0.5);
        wd = mix(wd, randomVec, 0.5);
        wd = normalize(wd);
        wcp += wd * (getNoise().w*1.0);
        wcp += randomVec*2.5;

        #if 0
            vec3 hitPos;
            vec3 hitNormal;
            if(raycastShadowVolume(wcp+normal*2.0, wd, 100.0, hitPos, hitNormal)){
                shadow = 0.0;
            }
        #else
            if(raycastShadowVolumeSparse(wcp+normal*0.5, wd, hitDist) < hitDist){
                shadow = 0.0;
            }
        #endif
    }


    //PBR Material
    float roughness = matl.r;
    float metallic = matl.g;
    //float emit = matl.b;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // ViewSpace
    vec3 V = -normalize(pos);
    vec3 N = (GetViewMatrix()*vec4(normal, 0.0)).xyz;
    vec3 L = (GetViewMatrix()*vec4(lightDir, 0.0)).xyz;

    float dist = distance(lightPos, pos);
    float attenuation =  clamp(light.Range-lightDistance, 0, 1) / pow(dist, light.Attenuation);

    vec3 radiance = light.Color*attenuation*shadow;
    vec3 Lo = PBRDirectLight(radiance, albedo, V, N, L, roughness, metallic);

    out_Color = vec4(Lo, 0.0);
}