#include "PhysicsSystem.h"

#include "World/Components.h"
#include "Editor/EUI/EUI.h"
#include "Asset/VoxAsset.h"

bool RayPlaneCast(
	const glm::vec3& planePos,
	const glm::vec3& planeNormal,
	const glm::vec3& rayPos,
	const glm::vec3& rayDir,
	float& t) {

	glm::vec3 delta = rayPos - planePos;
	float x = glm::dot(delta, planeNormal);
	float y = glm::dot(rayDir, planeNormal);

	if (y * x > 0.0f) {
		return false;
	}

	t = -(x / y);

	return true;
}

bool RayDoublePlaneCast(
	const glm::vec3& planePos,
	const glm::vec3& planeNormal,
	const float      planeDist,
	const glm::vec3& rayPos,
	const glm::vec3& rayDir,
	float& tMin,
	float& tMax) {

	glm::vec3 delta = rayPos - planePos;
	float x = glm::dot(delta, planeNormal); //Distance to plane
	float y = glm::dot(rayDir, planeNormal);//projected rayDir

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
	const glm::mat4& obb,
	const glm::vec3& obbSize,
	const glm::vec3& rayPos,
	const glm::vec3& rayDir,
	float& t
	) {

	glm::vec3 pos = obb[3];

	float greatestMin = -99999999.0f;
	float smallestMax = 99999999.0f;

	//TODO: Normalize Directions
	float xMin = 0.0f;
	float xMax = 0.0f;

	for (int i = 0; i < 3; i++) {
		if (RayDoublePlaneCast(pos, glm::normalize(obb[i]), obbSize[i]*glm::length(obb[i]), rayPos, rayDir, xMin, xMax)) {
			greatestMin = glm::max(greatestMin, xMin);
			smallestMax = glm::min(smallestMax, xMax);
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


bool RayVoxCast(
	const glm::mat4& obb,
	const glm::vec3& obbSize,
	AssetRefT<VoxAsset>& obbVox,
	const glm::vec3& rayPos,
	const glm::vec3& rayDir,
	float& t
) {
	glm::mat4 invObb = glm::inverse(obb);
	glm::vec3 localPos = (invObb * glm::vec4(rayPos, 1.0f))*10.0f;
	invObb[3] = glm::vec4(0,0,0,1);
	glm::vec3 localDir = invObb * glm::vec4(rayDir, 0.0f);

	//Fix aligned directions (zeroes in localDir)
	if (localDir.x == 0.0f)localDir.x = 0.00000001f;
	if (localDir.y == 0.0f)localDir.y = 0.00000001f;
	if (localDir.z == 0.0f)localDir.z = 0.00000001f;

	glm::vec3 stepSign = glm::sign(localDir);
	glm::vec3 tDelta = glm::vec3(1.0f,1.0f,1.0f) / (localDir*stepSign);
	glm::ivec3 currentVoxel = glm::ivec3(glm::floor(localPos));
	glm::vec3 nextVoxelBounds = glm::vec3(currentVoxel) + (stepSign*0.5f+0.5f);
	glm::vec3 tMax = (nextVoxelBounds - localPos) / localDir;

	int i = 0;
	do {
		glm::vec3 select = glm::step(tMax, glm::vec3(tMax.z, tMax.x, tMax.y)) * glm::step(tMax, glm::vec3(tMax.y, tMax.z, tMax.x));
		currentVoxel += select * stepSign;

		if (glm::clamp(currentVoxel, glm::ivec3(0, 0, 0), glm::ivec3(obbSize)-1) != currentVoxel) {
			return false;
		}

		uint8 voxel = *obbVox->PixelAt(currentVoxel.x, currentVoxel.y, currentVoxel.z);


		if (voxel != 0) {
			t = glm::dot(tMax, select)*0.1f;
			return true;
		}
		else {
			tMax += tDelta * select;
		}

	} while (i++ < 256);

	return false;
}

bool PhysicsSystem::RayCast(glm::vec3 start, glm::vec3 dir, float& hitt, entt::entity& hitEntity) {
	float bestt = 999999.0f;
	bool hit = false;
	
	R->view<Transform, VoxRenderer>().each([&](const entt::entity e, Transform& tr, VoxRenderer& v) {
		if (!v.Vox.IsValid())return;

		//TODO: Remove this
		// This disables the Character collision raycast test but it's slow
		entt::entity te = e;
		do {
			if (R->has<Character>(te)) {
				return;
			}
			else {
				te = W->GetParent(te);
			}
		}while(te != entt::null);

		auto s = v.Vox->GetImage().getExtent();

		glm::mat4 worldMatrix = tr.WorldMatrix;
		worldMatrix = glm::translate(worldMatrix, -v.Pivot);

		glm::vec3 size = glm::ivec3(s.width, s.height, s.depth);
		float t = 0.0f;
		if (RayOBBCast(worldMatrix, size*0.1f, start, dir, t)) {
			t = glm::max(t, 0.0f);// Prevent from casting from the start of the obb, which can be behind the camera
			float t2 = 0.0f;
			if (RayVoxCast(worldMatrix, size, v.Vox, start+dir*(t-0.001f), dir, t2)) {
				if (t+t2 < bestt) {
					bestt = t + t2;
					hitEntity = e;
				}
				hit = true;
			}
		}
	});
	
	hitt = bestt;
	return hit;
}
