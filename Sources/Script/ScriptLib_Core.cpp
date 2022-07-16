#include "ScriptLib_Core.h"


#include "glm/vec3.hpp"
#include "Core/Engine.h"
#include "World/Systems/ScriptSystem.h"
#include "World/World.h"
#include "World/Components.h"
#include <entt/entt.hpp>


void ScriptLib_Core::Import(ScriptVM& context) {
	context.Module("Core")
		.Class<entt::entity>("Entity")
			.Ctor()
#define DATA_TRANSFORM() \
		auto* sc = ScriptVM::From(vm); \
		auto& R = sc->GetUserData<WorldCtx>()->world->GetRegistry(); \
		entt::entity e = *sc->GetForeign<entt::entity>(0);
#define TRANSFORM_FIELD_AMOUNT(field) \
		double amount = sc->GetDouble(1); \
		Transform& t = R.get<Transform>(e); \
		t.field += amount; \
		R.replace<Transform>(e, t);

			.Method("rotate_x(_)", [](WrenVM* vm) {
				wrenEnsureSlots(vm, 2);
				DATA_TRANSFORM();
				TRANSFORM_FIELD_AMOUNT(Rotation.x);
			})
			.Method("rotate_y(_)", [](WrenVM* vm) {
				wrenEnsureSlots(vm, 2);
				DATA_TRANSFORM();
				TRANSFORM_FIELD_AMOUNT(Rotation.y);
			})
			.Method("rotate_z(_)", [](WrenVM* vm) {
				wrenEnsureSlots(vm, 2);
				DATA_TRANSFORM();
				TRANSFORM_FIELD_AMOUNT(Rotation.z);
			})
			.Method("move_x(_)", [](WrenVM* vm) {
				wrenEnsureSlots(vm, 2);
				DATA_TRANSFORM();
				TRANSFORM_FIELD_AMOUNT(Position.x);
			})
			.Method("move_y(_)", [](WrenVM* vm) {
				wrenEnsureSlots(vm, 2);
				DATA_TRANSFORM();
				TRANSFORM_FIELD_AMOUNT(Position.y);
			})
			.Method("move_z(_)", [](WrenVM* vm) {
				wrenEnsureSlots(vm, 2);
				DATA_TRANSFORM();
				TRANSFORM_FIELD_AMOUNT(Position.z);
			})
		.End()
		.Class("Engine")
			.StaticMethod("GetTime()", [](WrenVM* vm) { wrenSetSlotDouble(vm, 0, Engine::GetTime()); })
		.End()
		.Class("Input")
				.StaticMethod("is_w_down", [](WrenVM* vm) { wrenSetSlotBool(vm, 0, Input::IsKeyPressed(Key::W)); })
				.StaticMethod("is_s_down", [](WrenVM* vm) { wrenSetSlotBool(vm, 0, Input::IsKeyPressed(Key::S)); })
				.StaticMethod("is_a_down", [](WrenVM* vm) { wrenSetSlotBool(vm, 0, Input::IsKeyPressed(Key::A)); })
				.StaticMethod("is_d_down", [](WrenVM* vm) { wrenSetSlotBool(vm, 0, Input::IsKeyPressed(Key::D)); })
		.End()
		.Class<glm::vec3>("Vec3")
			.Ctor()
			.Field<&glm::vec3::x>("x")
			.Field<&glm::vec3::y>("y")
			.Field<&glm::vec3::z>("z")
		.End();
}
