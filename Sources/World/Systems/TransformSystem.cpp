#include "TransformSystem.h"

#include "World/World.h"

void DirtyFlagHierarchy(World& W, entt::entity e) {
    W.GetRegistry().emplace_or_replace<Dirty>(e);

    W.ForEachChild(e, [&](entt::entity child) { DirtyFlagHierarchy(W, child); });
}
void TransformHierarchy(World& W, entt::entity e, glm::mat4 parentMatrix = glm::mat4(1.0f)) {
    Transform& t = W.GetRegistry().get<Transform>(e);

    W.Transform->RealculateMatrix(e, t, parentMatrix);

    W.ForEachChild(e, [&](entt::entity child) { TransformHierarchy(W, child, t.WorldMatrix); });
}

void TransformSystem::OnTransformCreated(entt::registry& r, entt::entity e) {
    r.emplace_or_replace<Dirty>(e);
}
void TransformSystem::OnTransformUpdated(entt::registry& r, entt::entity e) {
    DirtyFlagHierarchy(*W, e);
}

void TransformSystem::OnCreate() {
    R->on_construct<Transform>().connect<&TransformSystem::OnTransformCreated>(this);
    R->on_update<Transform>().connect<&TransformSystem::OnTransformUpdated>(this);
}

void TransformSystem::OnUpdate(float dt) {
    R->view<Transform, Changed>().each([&](const entt::entity e, Transform& t) {
        t.PreviousWorldMatrix = t.WorldMatrix;  // TODO: Dirty -> Changed Tag
    });
    R->clear<Changed>();

    R->sort<Dirty>([&](const entt::entity a, const entt::entity b) {
        // TODO: Correct Sorting of Hierarchy
        /*
        bool x = false;

        if (R->has<Hierarchy>(a) && R->has<Hierarchy>(b)) {
                const auto& l = R->get<Hierarchy>(a);
                const auto& r = R->get<Hierarchy>(b);

                x |= l.Parent == b;
                x |= r.Parent == a;
        }
        */

        return R->entity(a) < R->entity(b);
    });
    R->view<Dirty>().each([&](const entt::entity e) {
        R->emplace<Changed>(e);

        // TODO: Optimize
        if (R->has<Hierarchy>(e) && R->get<Hierarchy>(e).Parent != entt::null && R->has<Transform>(R->get<Hierarchy>(e).Parent)) {
            TransformSystem::RealculateMatrix(e, R->get<Transform>(e), R->get<Transform>(R->get<Hierarchy>(e).Parent).WorldMatrix);
        } else {
            TransformSystem::RealculateMatrix(e, R->get<Transform>(e));
        }
    });
    R->clear<Dirty>();
}

glm::mat4& TransformSystem::GetWorldMatrix(entt::entity e) {
    if (R->has<Dirty>(e)) {
        RealculateMatrix(e, R->get<Transform>(e), R->has<Hierarchy>(e) ? GetWorldMatrix(R->get<Hierarchy>(e).Parent) : glm::identity<glm::mat4>());
        R->remove<Dirty>(e);
    }
    return R->get<Transform>(e).WorldMatrix;
}

glm::vec3 TransformSystem::GetWorldPosition(entt::entity e) {
    return glm::vec3(GetWorldMatrix(e)[3]);
}
void TransformSystem::SetWorldPosition(entt::entity e, glm::vec3 pos) {
    Transform& t = R->get<Transform>(e);
    if (R->has<Hierarchy>(e)) {
        auto inverseParentMatrix = glm::inverse(GetWorldMatrix(R->get<Hierarchy>(e).Parent));
        t.Position = inverseParentMatrix * glm::vec4(pos, 1.0f);
    } else {
        t.Position = pos;
    }
    R->replace<Transform>(e, t);
}

glm::quat TransformSystem::GetWorldRotation(entt::entity e) {
    return glm::quat_cast(GetWorldMatrix(e));
}
void TransformSystem::SetWorldRotation(entt::entity e, glm::quat rot) {
    auto& t = R->get<Transform>(e);

    if (R->has<Hierarchy>(e)) {
        auto inverseParentRot = glm::inverse(glm::quat_cast(GetWorldMatrix(R->get<Hierarchy>(e).Parent)));
        t.Rotation = glm::eulerAngles(inverseParentRot * rot);
    } else {
        t.Rotation = glm::eulerAngles(rot);
    }

    R->replace<Transform>(e, t);
}

glm::vec3 TransformSystem::WorldToLocal(glm::vec3 worldPoint, entt::entity space) {
    glm::mat4& m = GetWorldMatrix(space);
    glm::vec4 r = glm::inverse(m) * glm::vec4(worldPoint, 1.0f);
    return glm::vec3(r);
}
glm::vec3 TransformSystem::LocalToWorld(glm::vec3 localPoint, entt::entity space) {
    glm::mat4& m = GetWorldMatrix(space);
    glm::vec4 r = m * glm::vec4(localPoint, 1.0f);
    return glm::vec3(r);
}

void TransformSystem::RealculateMatrix(entt::entity e, Transform& Trans, glm::mat4 ParentTransform) {
    Trans.Matrix = glm::identity<glm::mat4>();
    Trans.Matrix = glm::translate(Trans.Matrix, Trans.Position);

    Trans.Matrix = glm::rotate(Trans.Matrix, Trans.Rotation.z, glm::vec3(0, 0, 1));
    Trans.Matrix = glm::rotate(Trans.Matrix, Trans.Rotation.y, glm::vec3(0, 1, 0));
    Trans.Matrix = glm::rotate(Trans.Matrix, Trans.Rotation.x, glm::vec3(1, 0, 0));

    Trans.Matrix = glm::scale(Trans.Matrix, Trans.Scale);

    Trans.WorldMatrix = ParentTransform * Trans.Matrix;
}
