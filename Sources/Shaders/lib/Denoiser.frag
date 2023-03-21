#ifndef DENOISER_H
#define DENOISER_H

float RadiusWeight(float radius, float size) {
    return (size - radius)/size;
}
float NormalWeight(vec3 normal, vec3 nNormal, float depth) {
    return 1.0-clamp(exp(-max(dot(normal, nNormal), 0)*50.0 - 1.0e-7*depth), 0, 1);
}
float PlaneWeight(vec3 planePos, vec3 planeNormal, vec3 point, float depth) {
    return 1.0-clamp(abs(dot(point - planePos, planeNormal))*500.0/depth, 0.0, 1.0);
}
float IsInsideScreenWeight(vec2 uv) {
	return (clamp(uv, vec2(0), vec2(1)) == uv) ? 1.0 : 0.0;
}

#endif