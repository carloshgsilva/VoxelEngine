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

float RayCastAABB(vec3 pos, vec3 dir, vec3 aabb_min, vec3 aabb_max){
	vec3 delta_min = aabb_min - pos;
	vec3 delta_max = aabb_max - pos;

	vec3 t_min = delta_min/dir;
	vec3 t_max = delta_max/dir;

    vec3 smallest = min(t_min, t_max);
    vec3 biggest = max(t_min, t_max);

	smallest = max(smallest, vec3(0));

	float great_min = max(smallest.x, max(smallest.y, smallest.z));
	float small_max = min(biggest.x, min(biggest.y, biggest.z));

	return mix(9999999, great_min, step(0, small_max-great_min));
}

#endif