#ifndef BRDF_H
#define BRDF_H

#include "Math.frag"

struct Material {
    vec3 albedo;
    float emissive;
    float metallic;
    float roughness;
    float transmission;
};

struct BRDFSample {
    vec3 wi;
    vec3 brdf;
    float pdf;
};

void TBN(out vec3 T, out vec3 B, vec3 N) {
    vec3 up = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    T = normalize(cross(up, N));
    B = cross(N, T);
}
vec3 ToLocal(vec3 T, vec3 B, vec3 N, vec3 dir) {
    return normalize(vec3(dot(dir, T), dot(dir, B), dot(dir, N)));
}
vec3 ToWorld(vec3 T, vec3 B, vec3 N, vec3 dir) {
    return normalize(dir.x*T + dir.y*B + dir.z*N);
}

vec3 FresnelSchlick(vec3 f0, vec3 f90, float cos_theta) {
    return mix(f0, f90, pow(max(0.0, 1.0 - cos_theta), 5.0));
}
float GGXDistribution(float a2, float cos_theta) {
    float denom_sqrt = cos_theta * cos_theta * (a2 - 1.0) + 1.0;
    return a2 / (M_PI * denom_sqrt * denom_sqrt);
}
float GGXSmith(float ndotv, float a2) {
    float tan2_v = (1.0 - ndotv * ndotv) / (ndotv * ndotv);
    return 2.0 / (1.0 + sqrt(1.0 + a2 * tan2_v));
}
float GGXShadowing(float ndotv, float ndotl, float a2) {
    return GGXSmith(ndotv, a2) * GGXSmith(ndotl, a2);
} 
float GGXPdfVN(float a2, vec3 wo, vec3 h) {
    float G1 = GGXSmith(wo.z, a2);
    float D = GGXDistribution(a2, h.z);
    return G1 * D * max(0.0, dot(wo, h)) / wo.z;
}
// https://jcgt.org/published/0007/04/01/
// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
vec3 SampleGGXVNDF(vec3 Ve, float roughness, float U1, float U2) {
    float alpha_x = roughness;
    float alpha_y = roughness;

    // Section 3.2: transforming the view direction to the hemisphere configuration
    vec3 Vh = normalize(vec3(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));
    // Section 4.1: orthonormal basis (with special case if cross product is zero)
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    vec3 T1 = lensq > 0 ? vec3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : vec3(1,0,0);
    vec3 T2 = cross(Vh, T1);
    // Section 4.2: parameterization of the projected area
    float r = sqrt(U1);
    float phi = 2.0 * M_PI * U2;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.z);
    t2 = (1.0 - s)*sqrt(1.0 - t1*t1) + s*t2;
    // Section 4.3: reprojection onto hemisphere
    vec3 Nh = t1*T1 + t2*T2 + sqrt(max(0.0, 1.0 - t1*t1 - t2*t2))*Vh;
    // Section 3.4: transforming the normal back to the ellipsoid configuration
    vec3 Ne = normalize(vec3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0, Nh.z)));
    return Ne;
}

bool SampleBRDFSpecular(inout BRDFSample smp, out vec3 F, vec3 V, vec3 N, vec3 albedo, float roughness, float metallic, vec2 rnd){
    vec3 T, B;
    TBN(T, B, N);
    vec3 wo = ToLocal(T, B, N, V);

    float a2 = roughness*roughness;
    
    vec3 ni = SampleGGXVNDF(-wo, roughness, rnd.x, rnd.y);
    float ggx_pdf = GGXPdfVN(a2, -wo, ni);
    vec3 wi = reflect(wo, ni);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    F = FresnelSchlick(F0, vec3(1.0), dot(ni, wi));
    float D = GGXDistribution(a2, ni.z);
    float G2 = GGXShadowing(wo.z, wi.z, a2);
    
    smp.wi = ToWorld(T, B, N, wi);

    #if 1
        float G2_over_G1 = GGXSmith(wi.z, a2);
        smp.brdf = F * G2_over_G1;
        smp.pdf = 1.0;
    #else
        smp.brdf.value = wi.z * F * D * G2 / (4.0 * -wo.z * wi.z);
        smp.brdf.pdf = ggx_pdf / (4.0 * dot(-wo, ni));
    #endif
    
    return smp.pdf > 0.0;
}
vec3 SampleCosineHemisphere(vec3 direction, vec2 rand) {
    float theta = 6.2831853 * rand.x;
    float u = 2.0 * rand.y - 1.0;
    float r = sqrt(1.0 - u * u);
    return normalize(direction + vec3(r * cos(theta), r * sin(theta), u));
}
vec3 SampleUniformHemisphere(vec3 N, vec2 rand) {
    float r = sqrt(1.0 - rand.x * rand.x);
    float phi = 2.0 * M_PI * rand.y;
    
    vec3 B = normalize(cross(N, vec3(0.3,0.4,0.5)));
    vec3 T = cross(B, N);
    
    return normalize(r * sin(phi) * B + rand.x * N + r * cos(phi) * T);
}

bool SampleBRDFDiffuse(inout BRDFSample smp, vec3 V, vec3 N, vec3 albedo, float metallic, vec2 rnd) {
    vec3 dir = SampleCosineHemisphere(N, rnd);

    smp.brdf = vec3(albedo);
    smp.pdf = 1.0;
    smp.wi = dir;

    return true;
}

bool SampleBRDF(out BRDFSample smp, in Material mat, vec3 V, vec3 N, vec4 rnd) {
    SampleBRDFDiffuse(smp, V, N, mat.albedo, mat.metallic, rnd.xy);
    return true;

    vec3 Fspecular = vec3(1);
    BRDFSample specular;
    SampleBRDFSpecular(specular, Fspecular, V, N, mat.albedo, max(mat.roughness, 0.001), mat.metallic, rnd.xy);

    
    float specularP = max(Fspecular.x, max(Fspecular.y, Fspecular.z));
    float diffuseP = (1.0 - specularP);

    smp = specular;
    if(specularP > rnd.x) {
        //smp.pdf *= specularP;
    } else {
        SampleBRDFDiffuse(smp, V, N, mat.albedo, mat.metallic, rnd.xy);
        //smp.pdf *= diffuseP;
    }

    return smp.pdf > 0;
}

#endif