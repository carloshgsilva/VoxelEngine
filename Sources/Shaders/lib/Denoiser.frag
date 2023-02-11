#ifndef DENOISER_H
#define DENOISER_H

float RadiusWeight(float radius, float size) {
    return (size - radius)/size;
}
float NormalWeight(vec3 normal, vec3 nNormal) {
    return 1.0-clamp(exp(-dot(normal, nNormal)*50.0), 0, 1);
}
float PlaneWeight(vec3 planePos, vec3 planeNormal, vec3 point) {
    return 1.0-clamp(abs(dot(point - planePos, planeNormal))*30.0, 0.0, 1.0);
}
float IsInsideScreenWeight(vec2 uv) {
	return (clamp(uv, vec2(0), vec2(1)) == uv) ? 1.0 : 0.0;
}

#endif