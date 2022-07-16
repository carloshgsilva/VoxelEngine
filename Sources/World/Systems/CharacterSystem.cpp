#include "CharacterSystem.h"

#include "World/Components.h"
#include "World/Systems/TransformSystem.h"
#include "World/Systems/PhysicsSystem.h"

glm::vec3 bezier(glm::vec3 a, glm::vec3 b, glm::vec3 c, float t) {
	glm::vec3 x = glm::lerp(a, b, t);
	glm::vec3 y = glm::lerp(b, c, t);
	return glm::lerp(x, y, t);
}


void CharacterSystem::AttractEntity(entt::entity e, glm::vec3 target, float factor) {
	glm::vec3 world_pos = W->Transform->GetWorldPosition(e);
	glm::vec3 delta = target - world_pos;
	W->Transform->SetWorldPosition(e, world_pos + delta * factor);
}

void CharacterSystem::OnUpdate(float dt) {
	if (!W->IsSimulating())return;

	R->view<Transform, Character>().each([&](entt::entity e, Transform& t, Character& c) {

		if (c.LeftFootIK == entt::null || c.RightFootIK == entt::null) {
			c.HeadIK = W->FindEntity("ik/ik_head", e);
			c.BodyIK = W->FindEntity("ik/ik_body", e);
			c.LeftFootIK = W->FindEntity("ik/ik_left_foot", e);
			c.RightFootIK = W->FindEntity("ik/ik_right_foot", e);
			c.LeftHandIK = W->FindEntity("ik/ik_left_hand", e);
			c.RightHandIK = W->FindEntity("ik/ik_right_hand", e);
			return;
		}

		float current_velocity = glm::sqrt(c.Velocity.x * c.Velocity.x + c.Velocity.z * c.Velocity.z);

		glm::vec3 body_center = W->Transform->GetWorldPosition(e);
		glm::mat4 world_matrix = t.WorldMatrix;
		glm::vec3 dir_right = world_matrix[0];
		glm::vec3 dir_forward = world_matrix[2];

		float hit_tl;
		float hit_tr;
		glm::vec3 target_left_foot;
		glm::vec3 target_right_foot;

		//Find feet targets
		{
			glm::vec3 start_target_center = body_center + dir_forward * current_velocity * (c.Running ? 0.15f : 0.3f);
			hit_tl = 0.0f;
			entt::entity hit;

			//Left
			glm::vec3 pos = start_target_center + dir_right * 0.15f;
			glm::vec3 dir = glm::vec3(0, -1, 0);
			if (W->Physics->RayCast(pos, dir, hit_tl, hit)) {}
			target_left_foot = pos + dir * hit_tl;

			//Right
			pos = start_target_center - dir_right * 0.15f;
			hit_tr = 0.0f;
			if (W->Physics->RayCast(pos, dir, hit_tr, hit)) {}
			target_right_foot = pos + dir * hit_tr;
		}

		float STEP_TIME = c.Running ? 0.3f : 0.4f;
		const float MID_AIR_STEP_ELEVATION = 0.5f;
		const float HAND_WALKING_ELEVATION = 0.1f;
		const float HAND_WALKING_FRONT_ELEVATION = 0.1f;

		bool is_touching_ground = hit_tl <= 0.801f && hit_tr <= 0.801 && c.Velocity.y <= 0.00001f;
		bool is_grounded = hit_tl <= 1.5f && hit_tr <= 1.5f && c.Velocity.y <= 0.00001f;

		//If both foot have ground target and is not going up
		if (is_grounded) {
			if (c.IsInAir) {
				c.IsInAir = false;
				c.AirTime = 0.0f;
				c.RightFootB = c.RightFootA = target_right_foot;
				c.LeftFootB = c.LeftFootA = target_left_foot;
			}

			//Foot placement
			c.FootAirTime += dt;
			if (c.FootAirTime > STEP_TIME) {
				c.FootAirTime = 0.0f;
				c.IsRightFootUp = !c.IsRightFootUp;
				//Now is the foot that will start to rise
				if (c.IsRightFootUp) {
					c.RightFootB = c.RightFootA;
				}
				else {
					c.LeftFootB = c.LeftFootA;
				}
			}
			else {
				if (c.IsRightFootUp) {
					c.RightFootA = target_right_foot;
					glm::vec3 last_right_foot = c.Running ? c.RightFootB + glm::vec3(0, c.FootAirTime / STEP_TIME, 0) : c.RightFootB;
					glm::vec3 temp_center_pos = (c.RightFootA + c.RightFootB) * 0.5f + glm::vec3(0, glm::distance(c.RightFootA, c.RightFootB) * MID_AIR_STEP_ELEVATION, 0);
					glm::vec3 final_right_foot_pos = bezier(last_right_foot, temp_center_pos, c.RightFootA, c.FootAirTime / STEP_TIME);

					//AttractEntity(c.RightFootIK, final_right_foot_pos, (float)dt * 10.0f);
					W->Transform->SetWorldPosition(c.RightFootIK, final_right_foot_pos);

					//AttractEntity(c.LeftFootIK, c.LeftFootA, (float)dt * 10.0f);
					W->Transform->SetWorldPosition(c.LeftFootIK, c.LeftFootA);
				}
				else {
					c.LeftFootA = target_left_foot;
					glm::vec3 last_left_foot = c.Running ? c.LeftFootB + glm::vec3(0, c.FootAirTime / STEP_TIME, 0) : c.LeftFootB;
					glm::vec3 temp_center_pos = (c.LeftFootA + c.LeftFootB) * 0.5f + glm::vec3(0, glm::distance(c.LeftFootA, c.LeftFootB) * MID_AIR_STEP_ELEVATION, 0);
					glm::vec3 final_left_foot_pos = bezier(last_left_foot, temp_center_pos, c.LeftFootA, c.FootAirTime / STEP_TIME);

					//AttractEntity(c.LeftFootIK, final_left_foot_pos, (float)dt * 10.0f);
					W->Transform->SetWorldPosition(c.LeftFootIK, final_left_foot_pos);

					//AttractEntity(c.RightFootIK, c.RightFootA, (float)dt * 10.0f);
					W->Transform->SetWorldPosition(c.RightFootIK, c.RightFootA);
				}

				//Body rotation
				{
					float time = (c.IsRightFootUp ? -1.0f : 1.0f) * glm::pi<float>() * c.FootAirTime / STEP_TIME;
					Transform& t = R->get<Transform>(c.BodyIK);
					t.Position.x = glm::sin(time) * current_velocity * 0.2f;
					R->replace<Transform>(c.BodyIK, t);
				}
			}

			//Hand swinging
			{
				float hand_time = glm::smoothstep(0.0f, 1.0f, c.FootAirTime / STEP_TIME);
				hand_time = c.IsRightFootUp ? 1.0f - hand_time : hand_time;

				glm::vec3 hands_center = body_center;
				glm::vec3 arm_swing = dir_forward * current_velocity * 0.3f;
				{//Left
					glm::vec3 left_hand_a = hands_center + dir_right * 0.4f + arm_swing + glm::vec3(0, current_velocity * HAND_WALKING_FRONT_ELEVATION * (c.Running ? 3.5f : 1.0f), 0);
					glm::vec3 left_hand_b = hands_center + dir_right * 0.4f - glm::vec3(0, current_velocity * (c.Running ? 0.5f : 0.0f), 0);
					glm::vec3 left_hand_c = hands_center + dir_right * 0.4f - arm_swing + glm::vec3(0, current_velocity * (c.Running ? 0.25f : 0.0f), 0);

					glm::vec3 final_left_hand_pos = bezier(left_hand_a, left_hand_b, left_hand_c, hand_time);

					AttractEntity(c.LeftHandIK, final_left_hand_pos, (float)dt * 10.0f);
					//W->Transform->SetWorldPosition(c.LeftHandIK, final_left_hand_pos);
				}
				{//Right
					glm::vec3 right_hand_a = hands_center - dir_right * 0.4f - arm_swing + glm::vec3(0, current_velocity * (c.Running ? 0.25f : 0.0f), 0);
					glm::vec3 right_hand_b = hands_center - dir_right * 0.4f - glm::vec3(0, current_velocity * (c.Running ? 0.5f : 0.0f), 0);
					glm::vec3 right_hand_c = hands_center - dir_right * 0.4f + arm_swing + glm::vec3(0, current_velocity * HAND_WALKING_FRONT_ELEVATION * (c.Running ? 3.5f : 1.0f), 0);

					glm::vec3 final_right_hand_pos = bezier(right_hand_a, right_hand_b, right_hand_c, hand_time);

					AttractEntity(c.RightHandIK, final_right_hand_pos, (float)dt * 10.0f);
					//W->Transform->SetWorldPosition(c.RightHandIK, final_right_hand_pos);
				}
			}

			//Height Asjustment
			t.Position.y += (glm::min(c.LeftFootA.y, c.RightFootA.y) + 0.8f - t.Position.y) * dt * 10.0f;
			c.Velocity.y -= c.Velocity.y * (float)dt * 10.0f;

		}
		//At least one foot doesn't have target
		else {
			if (c.IsInAir) {
				c.AirTime += dt;
				//Foot
				//Going Down
				if(c.Velocity.y < -0.001f){	
					AttractEntity(c.LeftFootIK, target_left_foot, (float)dt * 3.0f);
					AttractEntity(c.RightFootIK, target_right_foot, (float)dt * 3.0f);
				}
				//Going Up
				else {
					glm::vec3 start_target_center = body_center + glm::vec3(0,-0.6f,0);
					target_left_foot = start_target_center + dir_right * 0.15f;
					target_right_foot = start_target_center - dir_right * 0.15f;

					AttractEntity(c.LeftFootIK, target_left_foot, (float)dt * 10.0f);
					AttractEntity(c.RightFootIK, target_right_foot, (float)dt * 10.0f);
				}


				//Hands
				glm::vec3 hand_center = body_center + glm::vec3(0, 0.7f, 0);
				float co = glm::cos(c.AirTime * 15.0f);
				float si = glm::sin(c.AirTime * 15.0f);
				glm::vec3 left_hand_pos = hand_center + dir_right * 0.9f + dir_forward * co * 0.6f + glm::vec3(0, si * 0.6f, 0);
				AttractEntity(c.LeftHandIK, left_hand_pos, (float)dt * 10.0f);

				glm::vec3 right_hand_pos = hand_center - dir_right * 0.9f + dir_forward * co * 0.6f + glm::vec3(0, si * 0.6f, 0);
				AttractEntity(c.RightHandIK, right_hand_pos, (float)dt * 10.0f);

			}
			else {
				c.IsInAir = true;
			}
			
			//Gravity
			c.Velocity.y -= 15.0f * dt;
		}

		float WALK_SPEED = c.Running ? 20.0f : 10.0f;
		constexpr float AIR_RESISTANCE = 5.0f;

		auto mat = t.WorldMatrix;
		auto right = glm::vec3(1, 0, 0);//glm::vec3(mat[0]);
		auto forward = glm::vec3(0, 0, 1);//glm::vec3(mat[2]);

		//Movement
		 {
			//AWSD Movement
			glm::vec3 acceleration{ 0,0,0 };
			c.Running = Input::IsKeyDown(Key::LeftShift);
			if (is_grounded) {
				if (Input::IsKeyDown(Key::D)) { acceleration += right; }
				if (Input::IsKeyDown(Key::A)) { acceleration -= right; }
				if (Input::IsKeyDown(Key::S)) { acceleration += forward; }
				if (Input::IsKeyDown(Key::W)) { acceleration -= forward; }
				if (Input::IsKeyPressed(Key::Space) && is_touching_ground) { c.Velocity.y += 7.0f; }
			

				if (glm::length2(acceleration) > 0.000001f) {
					c.Velocity += glm::normalize(acceleration) * WALK_SPEED * ((float)dt);
				}

				//Air Resistance
				c.Velocity.x -= c.Velocity.x * (float)dt * AIR_RESISTANCE;
				c.Velocity.z -= c.Velocity.z * (float)dt * AIR_RESISTANCE;
				c.Velocity.y -= c.Velocity.y * (float)dt * AIR_RESISTANCE * 0.1f;
			}

			//Integrate Movement
			t.Position += c.Velocity * (float)dt;

			//Rotation
			if (glm::length(c.Velocity) >= 0.0001f) {
				float target_angle = glm::atan(c.Velocity.x, c.Velocity.z);
				glm::quat rot = glm::quat(t.Rotation);
				glm::quat target_rot = glm::quat(glm::vec3(0, target_angle, 0));

				rot = glm::slerp(rot, target_rot, 0.2f);
				glm::vec3 newRot = glm::eulerAngles(rot);


				//Head Tilting
				{
					glm::quat delta_rot = glm::inverse(rot) * target_rot;
					glm::vec3 delta_angle = glm::eulerAngles(delta_rot);

					Transform& t = R->get<Transform>(c.HeadIK);
					t.Position.x = delta_angle.y;
					t.Position.y = 10.0f;
					t.Position.z = current_velocity*0.3f;
					R->replace<Transform>(c.HeadIK, t);
				}

				t.Rotation = newRot;
			}
		}
		R->replace<Transform>(e, t);
	});
}
