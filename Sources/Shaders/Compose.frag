#version 450

#include "lib/Common.frag"

const vec3 SUN_DIR = normalize(vec3(0.3,0.4,0.5));

vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 filmic(vec3 x) {
  vec3 X = max(vec3(0.0), x - 0.004);
  vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return pow(result, vec3(2.2));
}

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _ColorTextureRID;
    int _DepthTextureRID;
    int _NormalTextureRID;
    int _LightTextureRID;
    int _ReflectionTextureRID;
    int _Bloom0TextureRID;
    int _Bloom1TextureRID;
    int _Bloom2TextureRID;
    int _Bloom3TextureRID;
    int _Bloom4TextureRID;
};

BINDING_VIEW_BUFFER()

layout(location=0) in struct {
    vec2 UV;
    vec3 FarVec;
} In;

layout(location=0) out vec4 out_Color;

#define PI 3.141592
#define iSteps 16
#define jSteps 8

vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}

vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {
    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    r = normalize(r);

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) return vec3(0,0,0);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0,0,0);
    vec3 totalMie = vec3(0,0,0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++) {

        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        // Increment the primary ray time.
        iTime += iStepSize;

    }

    // Calculate and return the final color.
    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

const float WATER_HEIGHT = 5.0;
const int WATER_DEPTH = 16;
const float FOG_HEIGHT = 100.0;
const float FOG_DENSITY = 5.0;

vec3 getSkyColor(vec3 e) {
    e.y = (max(e.y,0.0)*0.8+0.2)*0.8;
    return vec3(pow(1.0-e.y,2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * 1.1;
}

void main() {
    float exposure = 1.0;
    vec2 res = textureSize(DEPTH_TEXTURE, 0);

    vec2 uv = In.UV;
    float depth = texelFetch(DEPTH_TEXTURE, ivec2(uv*res), 0).r;
    vec4 albedo = texelFetch(COLOR_TEXTURE, ivec2(uv*res), 0);
    vec3 light = texelFetch(LIGHT_TEXTURE, ivec2 (uv*res), 0).rgb;
    vec3 reflection = texelFetch(REFLECTION_TEXTURE, ivec2(uv*res), 0).rgb;
    
    vec3 color = light+reflection*albedo.rgb;
    
    // Add Bloom
    color += texture(_BindingSampler2D[_Bloom0TextureRID], uv).rgb;
    color += texture(_BindingSampler2D[_Bloom1TextureRID], uv).rgb;
    color += texture(_BindingSampler2D[_Bloom2TextureRID], uv).rgb;
    color += texture(_BindingSampler2D[_Bloom3TextureRID], uv).rgb;
    color += texture(_BindingSampler2D[_Bloom4TextureRID], uv).rgb;
    
    //color = vec3(texture(LIGHT_TEXTURE, In.UV).a);
   
    //Water
    #if 0
        vec3 dir = normalize(In.FarVec);
        float t = (WATER_HEIGHT - GetCameraPosition().y)/In.FarVec.y;
        if(GetCameraPosition().y < WATER_HEIGHT){
            t = 0.0000001;
        }
        if(t > 0 && (t < depth || depth > 0.999 )){
            const float waterViewDepth = 1.5/FAR;
            vec3 waterPos = GetCameraPosition()+t*In.FarVec;
            float offset = (depth-t);

            //White foam
            hdrColor = mix(vec3(0.3), hdrColor, smoothstep(offset, 0.0, 0.00002));
            
            //Refraction
            hdrColor = mix(vec3(0,0.0,0.1), hdrColor, clamp(exp(-offset*5000.0), 0.0, 1.0));

            //Water Reflection (Just Sky for now)
            float fresnel = dot(normalize(In.FarVec), vec3(0,-1,0));
            fresnel = clamp(pow(fresnel, 0.1), 0, 1);
            hdrColor = mix(getSkyColor(normalize(reflect(In.FarVec, vec3(0,1,0)))), hdrColor, fresnel);

            float specular = dot(reflect(normalize(In.FarVec), vec3(0,1,0)), -SUN_DIR);
            depth = t;
        }
        depth = clamp(depth, 0, 1);
    #endif
/*
    vec3 atm = atmosphere(
            normalize(In.FarVec),           // normalized ray direction
            vec3(0,6372e3,0),               // ray origin
            vec3(1,-1,1)*100.0*SUN_DIR,      // position of the sun
            12.0,                           // intensity of the sun
            6371e3,                         // radius of the planet in meters
            6471e3,                         // radius of the atmosphere in meters
            vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient
            21e-6,                          // Mie scattering coefficient
            8e3,                            // Rayleigh scale height
            1.2e3,                          // Mie scale height
            0.758                           // Mie preferred scattering direction
        );
*/

    vec3 hdrColor = color;//pow(color, vec3(2.2));

    //Fog
    #if 0
        hdrColor = mix(hdrColor, vec3(100.0), depth*depth*FOG_DENSITY);
    #endif

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    //mapped = filmic(mapped);
    mapped = aces(mapped);

    // gamma correction 
    mapped = pow(mapped, vec3( 1.0 / 2.2));

    out_Color = vec4(mapped, albedo.a);
}