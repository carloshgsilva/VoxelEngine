#ifndef PATH_TRACE_H
#define PATH_TRACE_H

#include "View.frag"
#include "Light.frag"
#include "BRDF.frag"

const vec2 OFFSETS[64] = {
    vec2(0.5000, 0.3333),
    vec2(0.2500, 0.6667),
    vec2(0.7500, 0.1111),
    vec2(0.1250, 0.4444),
    vec2(0.6250, 0.7778),
    vec2(0.3750, 0.2222),
    vec2(0.8750, 0.5556),
    vec2(0.0625, 0.8889),
    vec2(0.5625, 0.0370),
    vec2(0.3125, 0.3704),
    vec2(0.8125, 0.7037),
    vec2(0.1875, 0.1481),
    vec2(0.6875, 0.4815),
    vec2(0.4375, 0.8148),
    vec2(0.9375, 0.2593),
    vec2(0.0313, 0.5926),
    vec2(0.5313, 0.9259),
    vec2(0.2813, 0.0741),
    vec2(0.7813, 0.4074),
    vec2(0.1563, 0.7407),
    vec2(0.6563, 0.1852),
    vec2(0.4063, 0.5185),
    vec2(0.9063, 0.8519),
    vec2(0.0938, 0.2963),
    vec2(0.5938, 0.6296),
    vec2(0.3438, 0.9630),
    vec2(0.8438, 0.0123),
    vec2(0.2188, 0.3457),
    vec2(0.7188, 0.6790),
    vec2(0.4688, 0.1235),
    vec2(0.9688, 0.4568),
    vec2(0.0156, 0.7901),
    vec2(0.5156, 0.2346),
    vec2(0.2656, 0.5679),
    vec2(0.7656, 0.9012),
    vec2(0.1406, 0.0494),
    vec2(0.6406, 0.3827),
    vec2(0.3906, 0.7160),
    vec2(0.8906, 0.1605),
    vec2(0.0781, 0.4938),
    vec2(0.5781, 0.8272),
    vec2(0.3281, 0.2716),
    vec2(0.8281, 0.6049),
    vec2(0.2031, 0.9383),
    vec2(0.7031, 0.0864),
    vec2(0.4531, 0.4198),
    vec2(0.9531, 0.7531),
    vec2(0.0469, 0.1975),
    vec2(0.5469, 0.5309),
    vec2(0.2969, 0.8642),
    vec2(0.7969, 0.3086),
    vec2(0.1719, 0.6420),
    vec2(0.6719, 0.9753),
    vec2(0.4219, 0.0247),
    vec2(0.9219, 0.3580),
    vec2(0.1094, 0.6914),
    vec2(0.6094, 0.1358),
    vec2(0.3594, 0.4691),
    vec2(0.8594, 0.8025),
    vec2(0.2344, 0.2469),
    vec2(0.7344, 0.5802),
    vec2(0.4844, 0.9136),
    vec2(0.9844, 0.0617),
    vec2(0.0078, 0.3951),
};

struct Sampler {
    ivec2 pixel;
    int frame;
    int depth;
};

void InitSampler(inout Sampler samp, ivec2 pixel, int frame) {
    samp.pixel = pixel;
    samp.frame = frame;
    samp.depth = 0;
}
#define BLUE_NOISE (Sampler2D[ViewBuffer[_ViewBufferRID].BlueNoiseRID])
vec4 SampleNoise(inout Sampler samp) {
    vec2 res = textureSize(BLUE_NOISE, 0);
    samp.depth++;
    //return texelFetch(BLUE_NOISE, ivec2((vec2(samp.pixel)+vec2(GOLDEN_RATIO*(mod(samp.frame+samp.depth*5,64)), GOLDEN_RATIO*(mod(samp.frame+samp.depth*7+1,64))))*res)%512, 0).r;
    return texelFetch(BLUE_NOISE, (samp.pixel+ivec2(OFFSETS[(samp.frame+samp.depth)%64]*512.0))%512, 0);
}


struct PathTraceState {
    vec3 o;
    vec3 d;
    vec3 n;
    vec3 throughput;
    vec3 acc;
};
void InitPathTraceState(out PathTraceState state, vec3 o, vec3 d) {
    state.o = o;
    state.d = d;
    state.n = vec3(0);
    state.throughput = vec3(1);
    state.acc = vec3(0);
}
bool PathTraceProceed(inout PathTraceState state, inout Sampler samp) {
    TraceHit hit;
    if(TraceRay(state.o, state.d, INF, hit)){
        Material mat;
        GetMaterial(hit.visibility, mat);
        
        // Sun Light
        vec3 skyDir = normalize(/*state.d+*/GetSunDir()*10.0);
        if(dot(skyDir, hit.normal) > 0.0 && !TraceShadowRay(state.o + state.d*hit.t + hit.normal*EPS*10.0, skyDir, INF)) {
            // TODO: correctly sample light
            state.acc += mat.albedo * state.throughput * GetSunColor() * max(dot(hit.normal, GetSunDir()), 0);
        }

        // Emissive
        state.acc += mat.albedo * state.throughput * mat.emissive * 10.0;

        BRDFSample smp;
        if(SampleBRDF(smp, mat, state.d, hit.normal, SampleNoise(samp))) {
            state.n = hit.normal;
            state.o = state.o + state.d*hit.t + hit.normal*EPS*max(hit.t, 10.0);
            state.d = smp.wi;
            state.throughput *= pow(smp.brdf, vec3(1/2.2));
            return true;
        }
    } else {
        state.n = -state.d;
        state.o += state.d * 1.0e12;
        state.acc += GetSkyColor(state.d) * state.throughput;
        return false;
    }

    return false;
}


#endif