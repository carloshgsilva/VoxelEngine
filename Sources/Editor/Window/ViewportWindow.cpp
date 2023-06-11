#include "ViewportWindow.h"

#include "Core/Engine.h"

#include "World/Systems/PhysicsSystem.h"
#include "World/Components.h"
#include "Mod/ModLoader.h"
#include "Util/MathUtil.h"

#include "Editor/EditorLayer.h"
#include "Editor/Importer/VoxImporter.h"
#include "Editor/EUI/EUI.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/ImGuizmo.h>
#include <imgui/imgui.h>

ViewportWindow::ViewportWindow() {
    _World = NewUnique<World>();
    Selection.SetWorld(_World.get());

    _Camera = NewUnique<EditorCamera>();
    _Camera->position = {26, 15, 25};
    _Camera->yaw = 0.81f;
    _Camera->pitch = -0.43f;

    _WorldRenderer = NewUnique<WorldRenderer>();
}

void ViewportWindow::DuplicateSelection() {
    Editor->CmdGroup([&] {
        World* W = GetWorld();
        auto& R = W->GetRegistry();

        // Copy entities so we can Remove and Add new ones per cmds
        std::vector<entt::entity> entities = Selection.GetEntities();

        // Clear Selections CMD
        Editor->Cmd([&] { Selection.Clear(); }, [&] { Selection.Clear(); });

        for (auto e : entities) {
            struct EntityDuplicateData {
                entt::entity src;
                entt::entity parent;
                entt::entity clone;
                PrefabAsset prefab;
            };
            Ref<EntityDuplicateData> data = std::make_shared<EntityDuplicateData>();
            data->src = e;
            data->prefab.FromWorld(W, e);
            data->parent = R.get<Hierarchy>(e).Parent;

            Editor->Cmd(
                [&R, this, W, data = data]() {
                    data->clone = data->prefab.Spawn(W, data->parent);
                    R.get<Hierarchy>(data->clone).Name += " (Clone)";
                    Selection.AddEntity(data->clone);
                },
                [&R, this, W, data = data]() { W->Destroy(data->clone); }, "Duplicate Entity");
        }
    });
}

void ViewportWindow::DeleteSelection() {
    Editor->CmdGroup([&] {
        World* W = GetWorld();
        auto& R = W->GetRegistry();

        // Copy entities so we can Remove and Add new ones per cmds
        std::vector<entt::entity> entities = Selection.GetEntities();

        // Clear Selections CMD
        Editor->Cmd([&] { Selection.Clear(); }, [&] { Selection.Clear(); });

        for (auto e : entities) {
            bool has_instance = R.has<Instance>(e);
            // Don't delete entity inside a Instance but can delete the Instance itself
            if (W->GetInstanceScope(e) != W->GetRoot() && !has_instance) {
                continue;
            }

            struct DeleteEntityData {
                std::string oldParentPath;
                AssetRefT<PrefabAsset> prefab;
                entt::entity oldEntity;
            };
            Ref<DeleteEntityData> data = std::make_shared<DeleteEntityData>();
            data->oldParentPath = W->GetEntityPath(R.get<Hierarchy>(e).Parent);

            if (has_instance) {
                data->prefab = R.get<Instance>(e).Prefab;
            } else {
                data->prefab = new PrefabAsset();
                data->prefab->FromWorld(W, e);
            }

            data->oldEntity = e;

            entt::entity parent = R.get<Hierarchy>(e).Parent;
            Editor->Cmd([&R, this, W, e, data = data] { W->Destroy(data->oldEntity); },
                        [&R, this, W, e, data = data] {
                            auto parent = W->FindEntity(data->oldParentPath);
                            data->oldEntity = data->prefab->Spawn(W, parent);
                            W->SetParent(data->oldEntity, parent);
                        },
                        "Delete Entity");
        }
    });
}

void ViewportWindow::RenderWorld() {
    if (_Width == 0 || _Height == 0) return;
    float dt = 0.016f;
    View view = _Camera->GetView(_Width, _Height);

    _WorldRenderer->DrawWorld(dt, view, *_World);
}

void ViewportWindow::OnEvent(Event& E) {
    _Camera->OnEvent(E);
    if (E.Is<FocusChangeEvent>()) {
        if (E.As<FocusChangeEvent>().Focused) {
            Editor->Viewport = this;
        }
    } else if (E.Is<ViewportChangeEvent>()) {
        Engine::OnBeforeUpdate([=] {
            _WorldRenderer->RecreateFramebuffer(_Width, _Height);
             OnUpdate(0.016f);
        });
    } else if (E.Is<KeyEvent>()) {
        auto& ke = E.As<KeyEvent>();
        if (ke.Press && !_Camera->IsMoving()) {
            if (ke.KeyCode == Key::Q) {
                _ManipulateType = ManipulateType::Translate;
            } else if (ke.KeyCode == Key::W) {
                _ManipulateType = ManipulateType::Rotate;
            } else if (ke.KeyCode == Key::E) {
                _ManipulateType = ManipulateType::Scale;
            }
        }
    } else if (E.Is<AssetImportedEvent>()) {
        Engine::OnBeforeUpdate([=] {
            AssetRefT<PrefabAsset> asset = Assets::Load(_World->GetLevel()->GetGUID());
            _World->SetLevel(asset);
        });
    }
}

// TODO: Move this to a TransformManipulator
static bool TransformManipulatorUsing{false};
struct EntityTransform {
    entt::entity e;
    glm::vec3 Position;
    glm::vec3 Rotation;
    glm::vec3 Scale;
    EntityTransform(entt::entity e, const Transform& t) {
        this->e = e;
        Position = t.Position;
        Rotation = t.Rotation;
        Scale = t.Scale;
    }
    void Apply(entt::registry& r) const {
        Transform t = r.get<Transform>(e);
        t.Position = Position;
        t.Rotation = Rotation;
        t.Scale = Scale;
        r.replace<Transform>(e, t);
    }
};
std::vector<EntityTransform> LastEntitiesTransform;
void ViewportWindow::OnGUI() {
    auto& R = _World->GetRegistry();

    bool windowOpen = true;
    ImGui::Begin(GetName().c_str(), &windowOpen, _IsDirty ? ImGuiWindowFlags_UnsavedDocument : 0);

    ImGui::BeginChild("##WorldRender");
    GUIInternal();

    if (Focused) {
        Editor->Viewport = this;

        _Camera->SetMoving(ImGui::IsMouseDown(ImGuiMouseButton_Right));

        //////////////
        // Hot-Keys //
        //////////////
        {
            // Crtl-S
            if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed((int)Key::S) && _IsDirty) {
                _World->GetLevel()->FromWorld(_World.get(), _World->GetRoot());
                Assets::SaveAsset(_World->GetLevel());
                _IsDirty = false;
            }
            // Crtl-D
            if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed((int)Key::D)) {
                DuplicateSelection();
            }
            // Delete
            if (ImGui::IsKeyPressed((int)Key::Delete)) {
                DeleteSelection();
            }
        }
    }

    View view = _Camera->GetView(_Width, _Height);
    EUI::SetViewport(view.ViewMatrix, view.ProjectionMatrix, ImGui::GetWindowPos(), ImGui::GetWindowSize(), ImGui::GetWindowDrawList());

    Image& SceneImage = _WorldRenderer->GetCurrentColorImage();
    if (_Width > 0 && _Height > 0) {
        ImGui::Image((ImTextureID)GetRID(SceneImage), ImVec2(_Width, _Height));
    }
    ImGui::SetCursorPos(ImVec2(8, 8));

    // Viewport Buttons
    {
        using namespace EUI;
        BeginOverlayGroup(1);
        {
            if (WorldButton(_playMode ? ICON_FA_STOP : ICON_FA_PLAY)) {
                _playMode = !_playMode;
                if (_playMode) {
                    Selection.Clear();

                    PrefabAsset prefab;
                    prefab.FromWorld(_World.get(), _World->GetRoot());
                    _prefabWorld = std::move(_World);
                    _World = NewUnique<World>();
                    prefab.Spawn(_World.get());
                    _World->SetSimulate(true);
                } else {
                    Selection.Clear();
                    Engine::OnBeforeUpdate([&] { _World = std::move(_prefabWorld); });
                }
            }
        }
        EndOverlayGroup();

        BeginOverlayGroup(3);
        {
            if (WorldButton(ICON_FA_ARROWS_ALT, _ManipulateType == ManipulateType::Translate)) {
                _ManipulateType = ManipulateType::Translate;
            }
            if (WorldButton(ICON_FA_SYNC, _ManipulateType == ManipulateType::Rotate)) {
                _ManipulateType = ManipulateType::Rotate;
            }
            if (WorldButton(ICON_FA_EXPAND_ARROWS_ALT, _ManipulateType == ManipulateType::Scale)) {
                _ManipulateType = ManipulateType::Scale;
            }
        }
        EndOverlayGroup();
    }

    // Manipulate Guizmo
    if (R.valid(Selection.GetEntity()) && R.has<Transform>(Selection.GetEntity()) && !_Camera->IsMoving()) {
        entt::entity selectedEntity = Selection.GetEntity();
        Transform& t = R.get<Transform>(selectedEntity);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, _Width, _Height);

        // IKChain Gizmos
        if (R.has<IKChain>(selectedEntity)) {
            IKChain& c = R.get<IKChain>(selectedEntity);
            if (c.Positions.size() > 0) {
                for (int i = 0; i < c.Positions.size() - 1; i++) {
                    EUI::DrawLine(c.Positions[i], c.Positions[i + 1]);
                }
                for (auto& p : c.Positions) {
                    EUI::DrawSphere(p, 0.1f);
                }
            }
        }

        // Bounds of PointLight Range
        if (R.has<Light>(Selection.GetEntity())) {
            Light& l = R.get<Light>(Selection.GetEntity());
            float r = l.Range;
            glm::vec3 pos = t.WorldMatrix[3];
            glm::vec3 lastPosX = pos + glm::vec3(0, 0, 1) * r;
            glm::vec3 lastPosY = pos + glm::vec3(0, 0, 1) * r;
            glm::vec3 lastPosZ = pos + glm::vec3(0, 1, 0) * r;
            for (float a = 0.0f; a <= glm::pi<float>() * 2.01f; a += glm::pi<float>() / 16.0f) {
                float s = glm::sin(a) * r;
                float c = glm::cos(a) * r;

                glm::vec3 posX = pos + glm::vec3(0, s, c);
                glm::vec3 posY = pos + glm::vec3(s, 0, c);
                glm::vec3 posZ = pos + glm::vec3(s, c, 0);
                EUI::DrawLine(posX, lastPosX);
                EUI::DrawLine(posY, lastPosY);
                EUI::DrawLine(posZ, lastPosZ);
                lastPosX = posX;
                lastPosY = posY;
                lastPosZ = posZ;
            }
        }

        // Bounds of VoxRenderer of all selected Entities
        /*
        for (const auto& e : Selection.GetEntities()) {
                if (R.has<VoxRenderer>(e)) {
                        VoxRenderer& vr = R.get<VoxRenderer>(e);

                        if (vr.Vox.IsValid()) {
                                Transform& t = R.get<Transform>(e);
                                auto size = vr.Vox->GetImage()->GetExtent3D();
                                EUI::DrawVoxBounds(t.WorldMatrix, glm::ivec3(size.width, size.height, size.depth));
                        }
                }
        }
        */

        // Manipualte operation
        ImGuizmo::OPERATION op = ImGuizmo::OPERATION::TRANSLATE;
        if (_ManipulateType == ManipulateType::Rotate) {
            op = ImGuizmo::OPERATION::ROTATE;
        } else if (_ManipulateType == ManipulateType::Scale) {
            op = ImGuizmo::OPERATION::SCALE;
        }

        glm::mat4 initialHandleMatrix = t.WorldMatrix;
        glm::mat4 handleMatrix = initialHandleMatrix;
        ImGuizmo::Manipulate(glm::value_ptr(view.ViewMatrix), glm::value_ptr(view.ProjectionMatrix), op, ImGuizmo::LOCAL, glm::value_ptr(handleMatrix));

        if (ImGuizmo::IsUsing() && Focused) {
            // Initial Entities Save Condition or Duplicate
            if (ImGui::IsMouseClicked(0)) {
                LastEntitiesTransform.clear();
                for (auto e : Selection.GetEntities()) {
                    LastEntitiesTransform.push_back({e, R.get<Transform>(e)});
                }
                TransformManipulatorUsing = true;
            }

            for (auto e : Selection.GetEntities()) {
                Transform t2 = R.get<Transform>(e);

                glm::mat4 transformSpaceMatrix = glm::inverse(initialHandleMatrix) * t2.WorldMatrix;
                glm::mat4 transformedMatrix = handleMatrix * transformSpaceMatrix;

                // inverseParentMatrix
                glm::mat4 toLocal = glm::inverse(t2.WorldMatrix * glm::inverse(t2.Matrix));

                glm::vec3 translation;
                glm::vec3 rotation;
                glm::vec3 scale;

                glm::mat4 newLocalMatrix = toLocal * transformedMatrix;

                MathUtil::DecomposeTransform(newLocalMatrix, translation, rotation, scale);

                glm::vec3 deltaRotation = rotation - t2.Rotation;

                t2.Position = translation;
                t2.Rotation += deltaRotation;
                t2.Scale = scale;

                R.replace<Transform>(e, t2);
            }
            if (ImGui::IsMouseClicked(0)) {
                if (ImGui::GetIO().KeyShift) {
                    DuplicateSelection();
                }
            }
        }

        if (ImGui::IsMouseReleased(0) && TransformManipulatorUsing) {
            TransformManipulatorUsing = false;
            std::vector<EntityTransform> DoEntitiesTransform;
            for (auto e : Selection.GetEntities()) {
                DoEntitiesTransform.push_back({e, R.get<Transform>(e)});
            }

            Editor->Cmd(
                [=, &R, newT = DoEntitiesTransform]() {
                    for (auto& et : newT) {
                        et.Apply(R);
                    }
                },
                [=, &R, oldT = LastEntitiesTransform]() {
                    for (auto& et : oldT) {
                        et.Apply(R);
                    }
                },
                "Transform");
        }
    }

    ImGui::EndChild();

    // Click to Select Entity
    if (Focused) {
        if (ImGui::IsItemClicked() && !ImGuizmo::IsUsing()) {
            float t;
            entt::entity e;
            if (_World->Physics->RayCast(_Camera->position, EUI::ScreenToWorld(ImGui::GetMousePos()), t, e)) {
                if (ImGui::GetIO().KeyCtrl) {
                    Selection.ToggleEntity(e);
                } else if (ImGui::GetIO().KeyShift) {
                    Selection.AddEntity(e);
                } else {
                    Selection.Clear();
                    Selection.AddEntity(e);
                }
            } else {
                if (!ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift) {
                    Selection.Clear();
                }
            }
        }
    }

    // Drag Asset Target
    {
        AssetGUID asset = EUI::DragAssetTarget(EUI::Drag_Asset_Prefab);
        if (asset != Asset::NullGUID) {
            AssetRefT<PrefabAsset> prefab = Assets::Load(asset);

            float t;
            entt::entity hitEntity;
            glm::vec3 pos = _Camera->position;
            glm::vec3 dir = EUI::ScreenToWorld(ImGui::GetMousePos());
            if (!_World->Physics->RayCast(pos, dir, t, hitEntity)) {
                t = 10.0f;
            }

            Log::info("Spawn Prefab");

            struct SpawnPrefabData {
                AssetRefT<PrefabAsset> prefab;
                glm::vec3 spawnPos;
                entt::entity spawnedEntity;
            };

            Ref<SpawnPrefabData> data = New<SpawnPrefabData>();
            data->prefab = prefab;
            data->spawnPos = pos + dir * t;

            Editor->Cmd(
                [&R, this, data = data]() {
                    data->spawnedEntity = data->prefab->Spawn(_World.get(), _World->GetRoot());
                    Transform& tr = R.get<Transform>(data->spawnedEntity);
                    tr.Position = data->spawnPos;
                    R.emplace_or_replace<Transform>(data->spawnedEntity, tr);
                },
                [&R, this, data = data]() { this->_World->Destroy(data->spawnedEntity); }, "Spawn Prefab");
        }
    }

    ImGui::End();

    if (!windowOpen) {
        Engine::OnBeforeUpdate([=]() { Editor->CloseViewport(this); });
    }
}

void ViewportWindow::OnUpdate(float dt) {
    if (Focused) {
        _Camera->Update(dt);
    }

    _World->Update(dt);

    // Render Selection as the WorldRenderer Outline
    auto& R = _World->GetRegistry();
    for (auto& e : Selection.GetEntities()) {
        if (!R.valid(e)) {
            Selection.Clear();
            return;
        }
        if (R.has<Transform>(e) && R.has<VoxRenderer>(e)) {
            auto& vr = R.get<VoxRenderer>(e);
            if (!vr.Vox.IsValid()) continue;
            glm::vec3 color = glm::vec3(0, 1, 0);
            glm::mat4 m = R.get<Transform>(e).WorldMatrix;

            m = glm::translate(m, -vr.Pivot);
            _WorldRenderer->CmdOutline(m, vr.Vox->GetImage(), color);
        }
    }
}

inline std::string ViewportWindow::GetName() {
    if (_prefabWorld && _prefabWorld->GetLevel().IsValid()) {
        return ModLoader::Get().GetAssetPath(_prefabWorld->GetLevel()->GetGUID());
    }
    if (_World->GetLevel().IsValid()) {
        return ModLoader::Get().GetAssetPath(_World->GetLevel()->GetGUID());
    }
    return "Viewport";
}
