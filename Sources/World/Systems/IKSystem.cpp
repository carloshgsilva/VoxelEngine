#include "IKSystem.h"

#include "World/Components.h"
#include "World/World.h"
#include "World/Systems/TransformSystem.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/orthonormalize.hpp>

/*
Bones are stored like this
[0,   1,   2,   3,   4   ]
[Tip, Mid, Mid, Mid, Root]

*/

glm::vec3 orthogonal(glm::vec3 v) {
	float x = abs(v.x);
	float y = abs(v.y);
	float z = abs(v.z);

	glm::vec3 other = x < y ? (x < z ? glm::vec3(1.0f, 0, 0) : glm::vec3(0, 1.0f, 0)) : (y < z ? glm::vec3(0, 1.0f, 0) : glm::vec3(0, 0, 1.0f));
	return glm::cross(v, other);
}
glm::quat get_rotation_between(glm::vec3 u, glm::vec3 v) {
	// It is important that the inputs are of equal length when
	// calculating the half-way vector.
	u = glm::normalize(u);
	v = glm::normalize(v);
	
	
	// Unfortunately, we have to check for when u == -v, as u + v
	// in this case will be (0, 0, 0), which cannot be normalized.
	if (u == -v)
	{
		// 180 degree rotation around any orthogonal vector
		return glm::quat(0, glm::normalize(orthogonal(u)));
	}
	

	glm::vec3 half = glm::normalize(u + v);
	return glm::quat(glm::dot(u, half), glm::cross(u, half));
}
//src: http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
glm::quat fromtwovectors(glm::vec3 u, glm::vec3 v) {
	glm::vec3 w = glm::cross(u, v);
	glm::quat q = glm::quat(glm::dot(u, v), w.x, w.y, w.z);
	q.w += q.length();
	return glm::normalize(q);

}

void IKSystem::OnChainUpdated(entt::registry& r, entt::entity e) {
	IKChain& c = r.get<IKChain>(e);
	c.Depth = glm::clamp(c.Depth, 3, 64);
}

void IKSystem::Reset(entt::entity e) {
	if (R->has<IKChain>(e)) {
		auto& c = R->get<IKChain>(e);
		c.Bones.clear();
		c.Positions.clear();
		c.Lengths.clear();
	}
}

void IKSystem::OnCreate() {
	R->on_construct<IKChain>().connect<&IKSystem::OnChainUpdated>(this);
	R->on_update<IKChain>().connect<&IKSystem::OnChainUpdated>(this);
}

float t = 0.0f;

void IKSystem::OnUpdate(DeltaTime dt) {
	t += dt;
	R->view<IKChain>().each([&](entt::entity e, IKChain& c) {
		int count = c.Depth;

		//Init
		if (c.Bones.size() != c.Depth) {
			c.Bones.clear();
			c.Positions.clear();
			c.Lengths.clear();

			int i = 0;
			entt::entity current = e;
			do  {
				if (i++ >= count)break;
				c.Bones.push_back(current);
				c.Positions.push_back(glm::vec3(0.0f));
				c.Lengths.push_back(glm::length(R->get<Transform>(current).Position));
			} while ((current = W->GetParent(current)) != entt::null);
		}

		count = c.Bones.size();

		auto root = c.Bones[count -1];
		auto target_entity = c.Target.GetEntity(W, root);
		auto target = glm::vec3(0.0f);
		auto pole_entity = c.Pole.GetEntity(W, root);
		glm::vec3 pole = glm::vec3(0.0f);

		//Get target position
		if (target_entity != entt::null) {
			target = W->Transform->GetWorldPosition(target_entity);
		}
		else {
			return;
		}

		//Pull positions to pole
		if (pole_entity != entt::null) {
			pole = W->Transform->GetWorldPosition(pole_entity);
			for (int i = 1; i < count -1; i++) { //don't need in the root nor tip
				glm::vec3 dir = pole - c.Positions[i];
				float l = glm::length(dir);
				if (l > 0.1f) {
					dir *= 0.1f / l;
				}
				c.Positions[i] += dir;
			}
		}

		//FABRIK
		for (int k = 0; k < 10;k++) {
			//Set Tip Transform
			if (target_entity != entt::null) {
				auto tip_bone = c.Bones[0];
				c.Positions[0] = target;
			}

			//Forward (tip -> chain)
			for (int i = 1; i < count - 1; i++) {
				auto a = c.Positions[i-1];
				auto b = c.Positions[i];
				auto dir = glm::normalize(b - a);
				if (glm::length(b - a) <= 0.0001f) {
					dir = glm::vec3(0, 1, 0);
				}
				c.Positions[i] = a + dir * c.Lengths[i-1];
			}

			//Set Root Position
			c.Positions[count - 1] = W->Transform->GetWorldPosition(root);

			//Backward (chain -> root)
			for (int i = count - 2; i > 0; i--) {
				auto a = c.Positions[i + 1];
				auto b = c.Positions[i];
				auto dir = glm::normalize(b - a);
				if (glm::length(b - a) <= 0.0001f) {
					dir = glm::vec3(0, 1, 0);
				}
				c.Positions[i] = a + dir * c.Lengths[i];
			}
		}

		//Get orthogonal from the center of the chain
		glm::vec3 up = glm::normalize(pole - (c.Positions[0] + c.Positions[count - 1]) * 0.5f);
		glm::orthonormalize(up, glm::normalize(c.Positions[0] - c.Positions[count - 1]));

		//Orientate bones
		entt::entity root_parent = W->GetParent(root);
		auto prevOrientation = root_parent == entt::null ? glm::identity<glm::quat>() : W->Transform->GetWorldRotation(root_parent);//TODO: Get Root's parent world Rotation
		for (int i = count -1; i > 0; i--) {
			auto bone1 = c.Bones[i];

			auto& t1 = R->get<Transform>(bone1);

			glm::vec3 target = c.Positions[i - 1];
			glm::vec3 src = c.Positions[i];
			glm::vec3 diff = src - target;

			glm::vec3 ortho = glm::normalize(glm::cross(target-src, pole-src));

			glm::quat local_angle_offset = get_rotation_between(glm::normalize(R->get<Transform>(c.Bones[i - 1]).Position), glm::vec3(0,0,1));

			glm::quat q = glm::inverse(prevOrientation)*glm::quatLookAt(glm::normalize(src - target), up)* local_angle_offset;
			prevOrientation = glm::normalize(prevOrientation*q);
			
			t1.Rotation = glm::eulerAngles(q);
			R->replace<Transform>(bone1, t1);
		}


		//Set Tip Transform
		entt::entity targetEntity = c.Target.GetEntity(W, root);
		if (targetEntity != entt::null) {
			auto tip_bone = c.Bones[0];
			auto world_rotation = W->Transform->GetWorldRotation(targetEntity);
			W->Transform->SetWorldRotation(tip_bone, world_rotation);
		}

	});
}
