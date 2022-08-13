#include "EditorLayer.h"

#include "Editor/Importer/AssetImporter.h"
#include "Editor/Window/HierarchyWindow.h"
#include "Editor/Window/ViewportWindow.h"
#include "Editor/Window/AssetsWindow.h"
#include "Editor/Window/PropertiesWindow.h"
#include "Editor/Util/AutoAssetImporter.h"
#include "Mod/ModLoader.h"
#include "Profiler/Profiler.h"

EditorLayer::EditorLayer() : Layer("Editor") {

	_Commands.SetOnCmd([this]() {
		this->Viewport->SetDirty(); 
	});

	_Assets = RegisterWindow(new AssetsWindow());
	_Hierarchy = RegisterWindow(new HierarchyWindow());
	_Properties = RegisterWindow(new PropertiesWindow());
	

	//AssetImporter::Import("Mods/default/ModernHouse.vox", "default");
	//AssetImporter::Import("Mods/default/FarmHouse.vox", "default");

	OpenViewport(Assets::Hash("default/ModernHouse.pf"));
	//OpenViewport(Assets::Hash("default/FarmHouse.pf"));
}

void EditorLayer::OpenViewport(AssetGUID asset) {
	
	// Don't open already opened prefabs
	for (ViewportWindow* vp : Viewports) {
		if (vp->GetWorld()->GetLevel().IsValid() && vp->GetWorld()->GetLevel()->GetGUID() == asset)return;
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
		}
		else {
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

	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z))) {
		_Commands.Undo();
	}
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y))) {
		_Commands.Redo();
	}

	//The Color is defaul Accent color so change it to white (because of the window titles color when docked)
	ImGui::PushStyleColor(ImGuiCol_Text, ImGuiRenderer::TextSecondary);

	//ImGui::ShowStyleEditor();

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

	//Simple frame time benchmark
	{
		if (Input::IsKeyPressed(Key::B)) {
			totalTime += ImGui::GetIO().DeltaTime * 1000.0f;
			count++;
		}
		if (Input::IsKeyPressed(Key::N)) {
			totalTime = 0.0f;
			count = 0.0f;
		}

		ImGui::Begin("Performance");
		ImGui::Text("ms: %f", ImGui::GetIO().DeltaTime * 1000.0f);
		ImGui::Text("fps: %f", 1.0f / ImGui::GetIO().DeltaTime);
		ImGui::Text("benchmark: %f", totalTime / count);

		ImGui::Checkbox("RayTrace", &Viewports[0]->GetWorldRenderer()->raytracing);
		ImGui::Checkbox("RayTrace GBuffer", &Viewports[0]->GetWorldRenderer()->raytraceGBuffer);

		if (GetTimestamps().size() > 0) {
			float scale = 0.0001f;
			for (auto& ts : GetTimestamps()) {
				ImVec2 cursor = ImGui::GetCursorScreenPos();
				cursor.x += 58.0f;
				ImVec2 min = { static_cast<float>(cursor.x + ts.start * scale), cursor.y };
				ImVec2 max = { static_cast<float>(cursor.x + ts.end * scale), min.y + ImGui::GetFontSize() };
				ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(255, 0, 0, 255));
				ImGui::GetWindowDrawList()->AddText(min, IM_COL32(255, 255, 255, 255), ts.name);
				ImGui::Text("%.2f", static_cast<float>(ts.end - ts.start) * 10e-6);
			}
		}

		ImGui::End();
	}
	
	
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
		//Update Viewports
		for (auto vp : Viewports) {
			vp->OnUpdate(dt);
		}
	}
	{
		PROFILE_SCOPE("Render Windows")
		//Render Gui

		if (Window::Get().GetWidth() != 0 && Window::Get().GetHeight() != 0) {
			if (this->Viewport != nullptr)this->Viewport->RenderWorld();
			_imguiRenderer.OnGUI([&] {
				OnGui();
			});
			CmdPresent([&] {
				_imguiRenderer.Draw();
			});
		}
		evk::Submit();
	}
}

void EditorLayer::OnEvent(Event& e) {
	e.Handle<ViewportChangeEvent>([&](ViewportChangeEvent& e) {
		OnUpdate(0.0f);
	});
	e.Handle<KeyEvent>([&](KeyEvent& e) {
		if(e.Press && e.KeyCode == Key::F11) {
			Window::Get().SetFullscreen(true);
		}
	});

	Viewport->OnEvent(e);

	//TODO: Put in some sort of EditorModule
	if (e.Is<DropFileEvent>()) {
		auto& E = e.As<DropFileEvent>();
		Log::info("[Debug] Files Dropped! Importing to: {}", _Assets->GetCurrentFolder());
		for (auto& f : E.Files) {
			if (AssetImporter::CanImport(f)) {
				Log::info("[Debug] Import File: {}", f);
				if (!AssetImporter::Import(f, _Assets->GetCurrentFolder())) {
					Log::error("[Debug] Error while importing the file: {}", f);
				}
			}else{
				Log::warn("[Debug] Can't import the file: {}", f);
			}
		}
		_Assets->Refresh();
	}
	
}
