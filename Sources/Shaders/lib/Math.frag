#ifndef MATH_H
#define MATH_H

#define M_PI 3.1415926535

bool IntersectRayAABB(vec3 o, vec3 d, vec3 aabb_min, vec3 aabb_max, out float out_t) {
    vec3 t_a = (aabb_min - o)/d;
    vec3 t_b = (aabb_max - o)/d;

    vec3 t_min = min(t_a, t_b);
    vec3 t_max = max(t_a, t_b);

    float near = max(max(t_min.x, max(t_min.y, t_min.z)), 0.0);
    float far = min(t_max.x, min(t_max.y, t_max.z));

    out_t = near;

    return near < far;
}

bool IntersectRayAABBNormal(vec3 o, vec3 d, vec3 aabb_min, vec3 aabb_max, out float out_t, out vec3 normal) {
    vec3 t_a = (aabb_min - o)/d;
    vec3 t_b = (aabb_max - o)/d;

    vec3 t_min = min(t_a, t_b);
    vec3 t_max = max(t_a, t_b);

    float near = max(max(t_min.x, max(t_min.y, t_min.z)), 0.0);
    float far = min(t_max.x, min(t_max.y, t_max.z));

    out_t = near;

    vec3 select = step(t_min.zxy, t_min.xyz) * step(t_min.yzx, t_min.xyz);
    normal = -select * sign(d);

    return near < far;
}

vec2 OctahedronWrap(vec2 v) {
	vec2 signVal;
	signVal.x = v.x >= 0.0 ? 1.0 : -1.0;
	signVal.y = v.y >= 0.0 ? 1.0 : -1.0;
	return (1.0 - abs(v.yx)) * signVal;
}
vec2 OctahedronEncode(vec3 n) {
	// https://twitter.com/Stubbesaurus/status/937994790553227264
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : OctahedronWrap(n.xy);
	n.xy = n.xy * 0.5 + 0.5;
	return n.xy;
}
vec3 OctahedronEncode(vec2 f) {
	// https://twitter.com/Stubbesaurus/status/937994790553227264
	f = f * 2.0 - 1.0;
	vec3 n = vec3(f.x, f.y, 1.0f - abs(f.x) - abs(f.y));
	float t = clamp(-n.z, 0.0, 1.0);
	n.x += n.x >= 0 ? -t : t;
	n.y += n.y >= 0 ? -t : t;
	return normalize(n);
}

#endif