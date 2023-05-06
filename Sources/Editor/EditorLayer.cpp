#include "EditorLayer.h"

#include "Editor/Importer/AssetImporter.h"
#include "Editor/Window/HierarchyWindow.h"
#include "Editor/Window/ViewportWindow.h"
#include "Editor/Window/AssetsWindow.h"
#include "Editor/Window/PropertiesWindow.h"
#include "Editor/Util/AutoAssetImporter.h"
#include "Mod/ModLoader.h"
#include "Profiler/Profiler.h"

static RuntimeProfiler timestamps = {};

EditorLayer::EditorLayer() : Layer("Editor") {
    _Commands.SetOnCmd([this]() { this->Viewport->SetDirty(); });

    _Assets = RegisterWindow(new AssetsWindow());
    _Hierarchy = RegisterWindow(new HierarchyWindow());
    _Properties = RegisterWindow(new PropertiesWindow());

    // AssetImporter::Import("Mods/default/ModernHouse.vox", "default");
    // AssetImporter::Import("Mods/default/FarmHouse.vox", "default");

    OpenViewport(Assets::Hash("default/ModernHouse.pf"));
    // OpenViewport(Assets::Hash("default/castle.pf"));
    //  OpenViewport(Assets::Hash("default/FarmHouse.pf"));
}

void EditorLayer::OpenViewport(AssetGUID asset) {
    // Don't open already opened prefabs
    for (ViewportWindow* vp : Viewports) {
        if (vp->GetWorld()->GetLevel().IsValid() && vp->GetWorld()->GetLevel()->GetGUID() == asset) return;
    }

    ViewportWindow* vp = RegisterWindow(new ViewportWindow());
    if (asset != Asset::NullGUID) {
        AssetRefT<PrefabAsset> pf = Assets::Load(asset);
        vp->GetWorld()->SetLevel(pf);
    }
    Viewport = vp;
    Viewports.push_back(vp);
}
void EditorLayer::CloseViewport(ViewportWindow* viewport) {
    auto index = std::find(Viewports.begin(), Viewports.end(), viewport);

    CHECK(index != Viewports.end());
    Viewports.erase(index);
    if (Viewport == viewport) {
        if (Viewports.size() > 0) {
            Viewport = Viewports[0];
        } else {
            OpenViewport();
        }
    }
    delete viewport;
}

void EditorLayer::OpenAsset(AssetGUID guid) {
    const std::string& path = ModLoader::Get().GetAssetPath(guid);
    if (path.find("pf") != -1) {
        OpenViewport(guid);
    }
}

float totalTime = 0.0f;
float count = 0.0f;
void EditorLayer::OnGui() {
    PROFILE_FUNC();
    
    for(auto& ts : evk::GetTimestamps()) {
        timestamps.Measure(ts.name, float(ts.end-ts.start));
    }

    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z))) {
        _Commands.Undo();
    }
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y))) {
        _Commands.Redo();
    }

    // The Color is defaul Accent color so change it to white (because of the window titles color when docked)
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiRenderer::TextSecondary);

    // ImGui::ShowStyleEditor();

    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Mod", "Crtl+O")) {
            }
            if (ImGui::MenuItem("New Mod", "Crtl+N")) {
            }
            if (ImGui::MenuItem("Save", "Crtl+S")) {
            }
            if (ImGui::MenuItem("Save As", "Crtl+Shift+S")) {
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) {
            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();

    ImGui::Begin(ICON_FA_ROCKET "  Performance");
    {
        /*
        auto& cap = Profiler::GetPreviousCapture();
        float start = float(cap.entries[0].begin);
        float finish = float(cap.entries[0].end);
        ImVec2 p = ImGui::GetCursorScreenPos();
        float SCALE = ImGui::GetContentRegionAvail().x / (finish - start);
        cap.Iterate([&](auto& e){
            ImVec2 a = {p.x + float(e.begin - start)*SCALE, p.y};
            ImVec2 b = {a.x + float(e.end - e.begin)*SCALE, a.y + ImGui::GetTextLineHeight()};
            ImGui::GetWindowDrawList()->AddRectFilled(a, b, ImGui::GetColorU32(ImGuiRenderer::Accent));
            
            
            ImVec2 oldCursor = ImGui::GetCursorScreenPos();
            ImVec2 t = {a.x + 4.0f, a.y};
            ImGui::SetCursorScreenPos(t);
            ImGui::Text("%s %.2f", e.name, float(e.end - e.begin));
            ImGui::SetCursorScreenPos(oldCursor);
            p.y += ImGui::GetTextLineHeight() + 2.0f;
            // ImGui::Text("%s %f -> %f", e.name, e.begin, e.end);
            // ImGui::Indent();
        }, [&](){
            p.y -= ImGui::GetTextLineHeight() + 2.0f;
            // ImGui::Unindent();
        });
        p.y += 150.0f;
        ImGui::SetCursorScreenPos(p);
        */
        
        Profiler::GetPreviousCapture().Clear();

        if(ImGui::BeginChild("CPU", {450.0f, 0.0f}, true)){
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_CheckMark]);
            ImGui::PushStyleColor(ImGuiCol_Separator, ImGuiRenderer::Border);
            ImGui::Text("CPU");
            ImGui::Separator();
            ImGui::PopStyleColor(2);
            {
                if (Input::IsKeyPressed(Key::B)) {
                    totalTime += Engine::GetDt() * 1000.0f;
                    count++;
                }
                if (Input::IsKeyPressed(Key::N)) {
                    totalTime = 0.0f;
                    count = 0.0f;
                }
                ImGui::Text("%.2fms (%.0fFPS)", Engine::GetDt() * 1000.0f, 1.0f / Engine::GetDt());
                if (count > 0) {
                    ImGui::Text("Average: %.2fms (%.0ffps)", totalTime / count, 1000.0f * count / totalTime);
                }
            }

            Engine::GetRuntimeProfiler().OnImGui(200.0f);
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if(ImGui::BeginChild("GPU", {300.0f, 0.0f}, true)){
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_CheckMark]);
            ImGui::PushStyleColor(ImGuiCol_Separator, ImGuiRenderer::Border);
            ImGui::Text("GPU");
            ImGui::Separator();
            ImGui::PopStyleColor(2);
            timestamps.OnImGui(100.0f);
        }
        ImGui::EndChild();

        ImGui::SliderInt("Sleep", &Engine::Get().sleepMS, 0, 1000);
        ImGui::SliderFloat("Time", &Viewports[0]->GetWorldRenderer()->timeOfDay, 0.0f, 24.0f);
        ImGui::Checkbox("Flow Time", &Viewports[0]->GetWorldRenderer()->timeFlow);

        ImGui::Separator();
        ImGui::Combo("Technique", (int*)&Viewports[0]->GetWorldRenderer()->technique, "PathTrace\0ReSTIR GI\0IR Cache\0", 3);
        ImGui::Combo("Output", (int*)&Viewports[0]->GetWorldRenderer()->outputImage, "Composed\0Diffuse\0Normal\0ReSTIR GI Radiance\0Specular\0", 4);
        ImGui::Checkbox("TAA", &Viewports[0]->GetWorldRenderer()->enableTAA);
        ImGui::Checkbox("Sub Pixel Jitter", &Viewports[0]->GetWorldRenderer()->enableJitter);
        ImGui::Checkbox("Samples Permutation", &Viewports[0]->GetWorldRenderer()->enablePermutation);

        ImGui::Separator();
        ImGui::Checkbox("Denoiser", &Viewports[0]->GetWorldRenderer()->enableDenoiser);
    }
    ImGui::End();

    _Assets->OnGUI();
    _Hierarchy->OnGUI();
    _Properties->OnGUI();
    for (auto viewport : Viewports) {
        viewport->OnGUI();
    }

    ImGui::PopStyleColor();
}

void EditorLayer::OnUpdate(float dt) {
    PROFILE_FUNC();

    AutoAssetImporter::CheckForImport();

    {
        PROFILE_SCOPE("Update Viewports")

        for (auto vp : Viewports) {
            vp->OnUpdate(dt);
        }
    }
    {
        PROFILE_SCOPE("Render Windows")

        if (Window::Get().GetWidth() != 0 && Window::Get().GetHeight() != 0) {
            if (this->Viewport != nullptr) this->Viewport->RenderWorld();
            _imguiRenderer.OnGUI([&] { OnGui(); });
            CmdTimestamp("ImGui", [&]{
                CmdPresent([&] {
                    _imguiRenderer.Draw();
                });
            });
        }
        evk::Submit();
    }
}

void EditorLayer::OnEvent(Event& e) {
    e.Handle<ViewportChangeEvent>([&](ViewportChangeEvent& e) { OnUpdate(0.0f); });
    e.Handle<KeyEvent>([&](KeyEvent& e) {
        if (e.Press && e.KeyCode == Key::F11) {
            Window::Get().FullscreenToggle();
        }
    });

    Viewport->OnEvent(e);

    // TODO: Put in some sort of EditorModule
    if (e.Is<DropFileEvent>()) {
        auto& E = e.As<DropFileEvent>();
        Log::info("[Debug] Files Dropped! Importing to: {}", _Assets->GetCurrentFolder());
        for (auto& f : E.Files) {
            if (AssetImporter::CanImport(f)) {
                Log::info("[Debug] Import File: {}", f);
                if (!AssetImporter::Import(f, _Assets->GetCurrentFolder())) {
                    Log::error("[Debug] Error while importing the file: {}", f);
                }
            } else {
                Log::warn("[Debug] Can't import the file: {}", f);
            }
        }
        _Assets->Refresh();
    }
}
