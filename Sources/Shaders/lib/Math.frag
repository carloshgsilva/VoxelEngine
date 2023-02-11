#ifndef MATH_H
#define MATH_H

bool RayDoublePlaneCast(
	vec3 planePos,
	vec3 planeNormal,
	float planeDist,
	vec3 rayPos,
	vec3 rayDir,
	out float tMin,
	out float tMax) {

	vec3 delta = rayPos - planePos;
	float x = dot(delta, planeNormal); //Distance to plane
	float y = dot(rayDir, planeNormal);//projected rayDir

	//If don't hit both planes
	if (y * x > 0.0f && y * (x - planeDist) > 0.0f) {
		return false;
	}

	float t1 = -(x / y);
	float t2 = -((x - planeDist) / y);

	if (t1 < t2) {
		tMin = t1;
		tMax = t2;
	}
	else {
		tMin = t2;
		tMax = t1;
	}

	return true;
}

bool RayOBBCast(
	mat4 obb,
	vec3 obbSize,
	vec3 rayPos,
	vec3 rayDir,
	out float t
	) {

	vec3 pos = vec3(obb[3]);

	float greatestMin = -99999999.0f;
	float smallestMax = 99999999.0f;

	//TODO: Normalize Directions
	float xMin = 0.0f;
	float xMax = 0.0f;

	for (int i = 0; i < 3; i++) {
		if (RayDoublePlaneCast(pos, normalize(vec3(obb[i])), obbSize[i]*length(obb[i]), rayPos, rayDir, xMin, xMax)) {
			greatestMin = max(greatestMin, xMin);
			smallestMax = min(smallestMax, xMax);
		}
		else {
			return false;
		}
	}

	//src: http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
	if (greatestMin < smallestMax) {
		t = greatestMin;
		return true;
	}
	else {
		return false;
	}
}

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

vec3 CosineSampleHemisphere(vec2 rand, vec3 direction) {
    float r = sqrt(rand.x);
    float theta = 6.2831853 * rand.y;
    float x = r * cos(theta);
    float y = r * sin(theta);

	vec3 s = vec3(x, y, sqrt(max(0.0, 1.0 - rand.x)));

	vec3 T = normalize(cross(vec3(1,0,0), direction));
	if(isnan(dot(T, vec3(1)))) T = normalize(cross(vec3(0,1,0), direction));
	vec3 B = normalize(cross(T, direction));
	vec3 N = direction;
 
    return normalize(s.x*T + s.y*B + s.z*N);
}

/*
vec3 CosineSampleHemisphere(vec2 rand, vec3 direction) {
	float theta = 6.2831853 * rand.x;
	float u = 2.0 * rand.y - 1.0;
	float r = sqrt(1.0 - u * u);

	return normalize(direction*0.99 + vec3(r * cos(theta), r * sin(theta), u));
}
*/

uint xorshift32(inout uint s) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint x = s;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return s = x;
}

uint InitRand(uint x, uint y) {
	uint sa = xorshift32(x);
	xorshift32(sa);
	xorshift32(sa);
	xorshift32(sa);
	uint sb = xorshift32(y);
	xorshift32(sb);
	xorshift32(sb);
	xorshift32(sb);
	return (sa+342165)^(sb+573458)+(sa+525246)*(sb+754373)+(sa+5643576)*(sb+346345);
}

float rand(inout uint s) {
	return float(xorshift32(s)) / float(0xFFFFFFFFu);
}

#endif