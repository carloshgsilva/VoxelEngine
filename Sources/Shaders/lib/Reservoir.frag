#ifndef RESERVOIR_H
#define RESERVOIR_H

#include "evk.frag"
#include "Math.frag"

struct GISample {
    vec3 xv, nv;
    vec3 xs, ns;
    vec3 radiance;
};

struct Reservoir {
    float w; // total weight
    float M; // sample count
    float W;

    GISample s;
};

void ReservoirStore(int b0, int b1, int b2, int b3, ivec2 coord, in Reservoir r) {
    imageStore(Image2DW[b0], coord, vec4(r.w, r.M, r.W, 0.0));
    imageStore(Image2DW[b1], coord, vec4(r.s.xv, uintBitsToFloat(PackNormal(r.s.nv))));
    imageStore(Image2DW[b2], coord, vec4(r.s.xs, uintBitsToFloat(PackNormal(r.s.ns))));
    imageStore(Image2DW[b3], coord, vec4(r.s.radiance, 0.0));
}

void ReservoirLoad(int b0, int b1, int b2, int b3, ivec2 coord, out Reservoir r) {
    vec4 d0 = imageLoad(Image2DR[b0], coord);
    vec4 d1 = imageLoad(Image2DR[b1], coord);
    vec4 d2 = imageLoad(Image2DR[b2], coord);
    vec4 d3 = imageLoad(Image2DR[b3], coord);

    r.w = d0.x;
    r.M = d0.y;
    r.W = d0.z;

    r.s.xv = d1.xyz;
    r.s.nv = UnpackNormal(floatBitsToUint(d1.w));
    r.s.xs = d2.xyz;
    r.s.ns = UnpackNormal(floatBitsToUint(d2.w));
    r.s.radiance = d3.xyz;
}

void ReservoirClear(inout Reservoir r) {
    r.w = 0.0;
    r.M = 0.0;
    r.W = 0.0;
}
void ReservoirUpdate(inout Reservoir r, in GISample s, float sW, float rand) {
    r.w += sW;
    r.M++;
    if(r.w == 0.0 || rand < sW / r.w)
        r.s = s;
}
void ReservoirMerge(inout Reservoir self, in Reservoir r, float pHat, float rand) {
    float M0 = self.M;
    ReservoirUpdate(self, r.s, pHat * r.W * r.M, rand);
    self.M = M0 + r.M;
}
void ReservoirClamp(inout Reservoir self, float count, float factor) {
    if(self.M > factor * count) {
        self.w *= factor * count / self.M;
        self.M = factor * count;
    }
}
void ReservoirFinalize() {
    
}

#endif