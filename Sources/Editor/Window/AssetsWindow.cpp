#include "AssetsWindow.h"

#include "Editor/EUI/EUI.h"
#include "Editor/EditorLayer.h"
#include "Editor/Importer/AssetImporter.h"
#include "Asset/PrefabAsset.h"

#include "World/Components.h"
#include "Mod/ModLoader.h"

#include "IO/SystemIO.h"

#include <entt/entt.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/IconsFontAwesome5.h>

AssetsWindow::AssetsWindow() {
	_CurrentFolder = (Mod::MODS/"default").generic_string();//TODO: Change this to something like Editor->ModName
	_FileExplorer.SetFolder(_CurrentFolder);
}

std::string AssetsWindow::GetCurrentFolder() { return std::filesystem::path(_CurrentFolder).lexically_relative(Mod::MODS).generic_string(); }

void AssetsWindow::RenderFolderIcon(ImVec2 min, ImVec2 max) {
	max.y -= 10;
	min.y += 10;

	min.x += 40;
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(155, 155, 155, 255), 4.0f);
	min.x -= 40;

	min.y += 8;
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(155, 155, 155, 255), 4.0f);
}

void AssetsWindow::RenderDeleteDialog() {
	if (!_DeletionPending)return;

	if (_DeletionBegin) {
		ImGui::OpenPopup(POPUP_DELETE);
		_DeletionBegin = false;
	}

	if (ImGui::BeginPopup(POPUP_DELETE)) {

		ImGui::Text("Are you sure you want to delete?");
		ImGui::Text(_SelectedItem.c_str());

		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
			_DeletionPending = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Delete")) {
			ImGui::CloseCurrentPopup();
			_DeletionPending = false;

			if (std::filesystem::is_directory(_SelectedItem)) {
				for (auto path : std::filesystem::recursive_directory_iterator(_SelectedItem)) {
					if (!std::filesystem::is_directory(path)) {
						Assets::DeleteAsset(Assets::Hash(path.path().lexically_relative(Mod::MODS).generic_string()));
					}
				}
				std::filesystem::remove_all(_SelectedItem);
			}
			else {
				Assets::DeleteAsset(Assets::Hash(std::filesystem::path(_SelectedItem).lexically_relative(Mod::MODS).generic_string()));
			}
			_FileExplorer.Invalidate();
		}

		ImGui::EndPopup();
	}
	else {
		Log::info("Closed Popup!");
		_DeletionPending = false;
	}
}

void AssetsWindow::RenderContextMenu() {
	if (ImGui::BeginPopup(POPUP_ASSET_MENU)) {

		if (AssetImporter::CanImport(_SelectedItem) && ImGui::MenuItem(ICON_FA_UPLOAD "   Import")) {
			AssetImporter::Import(_SelectedItem, std::filesystem::path(_CurrentFolder).lexically_relative(Mod::MODS).generic_string());
			_FileExplorer.Invalidate();
			ImGui::Separator();
		}

		if (ImGui::BeginMenu(ICON_FA_PLUS "   Create")) {
			if(ImGui::MenuItem(ICON_FA_FOLDER "   Folder")){
				std::filesystem::path targetItem = std::filesystem::path(_CurrentFolder) / "New Folder";
				std::filesystem::create_directory(targetItem);
				SelectItem(targetItem.generic_string());
				StartRenaming();
				_FileExplorer.Invalidate();
			}
			if(ImGui::MenuItem(ICON_FA_CUBE "   Prefab")){}
			ImGui::EndMenu();
		}

		ImGui::Separator();
		if (ImGui::MenuItem(ICON_FA_CLONE "   Clone")) {
			SelectedItemClone();
		}
		else if (ImGui::MenuItem(ICON_FA_COPY "   Copy Path")) {
			std::string targetItem = (std::filesystem::current_path() / GetTargetPath().make_preferred().string()).string();
			ImGui::SetClipboardText(targetItem.c_str());
		}
		else if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "   Open on File Explorer")) {
			std::filesystem::path path = GetTargetPath();
			std::string targetItem = path.make_preferred().string();
			bool isFolder = !path.has_extension();
			
			if (isFolder) {
				SystemIO::Run(std::string("explorer \"") + targetItem + std::string("\" &"));
			}
			else {
				SystemIO::Run(std::string("explorer /select,\"") + targetItem + std::string("\" &"));
			}
			
		}
		else if (ImGui::MenuItem(ICON_FA_PEN "   Rename", "F2")) {
			StartRenaming();
		}
		else if (ImGui::MenuItem(ICON_FA_TRASH "   Delete", "Del")) {
			SelectedItemDelete();
		}

		ImGui::EndPopup();
	}

}

//[Import] and Path
void AssetsWindow::RenderHeader() {
	ImGui::Button(ICON_FA_DOWNLOAD "  Import");

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f,0.25f,0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0.0f));
	{
		ImGui::SameLine(); ImGui::SetCursorPosX(258); ImGui::SetCursorPosY(ImGui::GetCursorPosY()-3);
		ImGui::Dummy(ImVec2(0, 0));

		int first = 0;
		int second = 0;
		char data[64] = {};
		do {
			second = _CurrentFolder.find('/', first);
			if (second == -1)second = _CurrentFolder.size();

			std::memcpy(data, _CurrentFolder.data() + first, second - first);
			data[second - first] = '\0';
			ImGui::SameLine();

			//All Others
			if (second < _CurrentFolder.size()) {
				if (ImGui::Button(data)) {
					_CurrentFolder = _CurrentFolder.substr(0, second);
				}
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), ICON_FA_ANGLE_RIGHT);
			}
			//Last One
			else {
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().FramePadding.x);
				ImGui::Text(data);
			}
			first = second + 1;

		} while (second < _CurrentFolder.size());

	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth()-200);

	ImVec2 oldCursor = ImGui::GetCursorScreenPos();
	ImGui::InputText("##EMPTY", _Filter, 32);

	if (!ImGui::IsItemActive() && _Filter[0] == '\0') {
		//Position the 'Search ... ' text inside the input
		oldCursor.x += 6.0f;
		oldCursor.y += 3.0f;
		ImGui::SetCursorScreenPos(oldCursor);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,0.3f));
		ImGui::Text(ICON_FA_SEARCH " Search ...");
		ImGui::PopStyleColor();
	}

	ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().FramePadding.x, ImGui::GetWindowContentRegionMin().y+ ImGui::GetStyle().FramePadding.y*2.0f+ImGui::GetFontSize() + 3.0f));
}

void AssetsWindow::RenderDirTree(DirEntry& entry) {
	if (entry.isFolder) {
		ImGui::PushID(entry.Path.c_str());
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (entry.Path == _CurrentFolder) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (!_FileExplorer.HasChildFolder(entry)) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		bool isOpen = ImGui::TreeNodeEx(entry.Path.c_str(), flags, "%s %s", ICON_FA_FOLDER, entry.Name.c_str());
		
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			_CurrentFolder = entry.Path;
		}
		
		if (isOpen) {
			_FileExplorer.Explore(entry, [&](DirEntry childEntry) {
				RenderDirTree(childEntry);
			});

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

}

void AssetsWindow::RenderDirContent(DirEntry& entry) {
	const float WIDTH = 80.0f;
	const float TEXT_HEIGHT = ImGui::GetFontSize()*2.0f;
	const float HEIGHT = WIDTH + TEXT_HEIGHT;
	const float SPACING = 8.0f;
	const float HOVER_SPACING = 4.0f;

	int columnCount = ImGui::GetContentRegionAvail().x / (WIDTH + SPACING*0.5f);
	int sameLine = 0;
	float offsetX = SPACING;
	float offsetY = SPACING;

	//TODO: Folders first then Files?
	//Folders
	_FileExplorer.Explore(entry, [&](DirEntry& childEntry) {
		//if (childEntry.isFolder) {
			
			//Filter
			//TODO: Improve with lowerCase and cache the file name before
			if (_Filter[0] != '\0' && childEntry.RelativePath.find(_Filter, childEntry.RelativePath.find_first_of('/')) == -1) {
				return;
			}

			bool isSelected = _SelectedItem == childEntry.Path.c_str();

			//New Line	
			if (sameLine >= columnCount) {
				sameLine = 0;
				offsetX = SPACING;
				offsetY += HEIGHT+SPACING*2.0f;
				ImGui::Dummy(ImVec2(0, SPACING));
			}

			ImGui::SetCursorPos(ImVec2(offsetX, offsetY));

			//Draw Hover Bounds
			ImVec2 groupMin = ImGui::GetCursorScreenPos();
			ImVec2 groupMax = groupMin;
			groupMax.x += WIDTH;
			groupMax.y += HEIGHT;

			ImGui::PushID(childEntry.Path.c_str());

			//Draw Widget
			ImGui::BeginGroup();
				
				//Thumbnail
				//TODO: Use an actual thumbnail
				ImGui::Dummy(ImVec2(WIDTH, WIDTH));
				if (childEntry.isFolder) {
					RenderFolderIcon(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				}
				else {
					ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 255, 255, 50), 4.0f);
				}
				
				//Offset -WIDTH for Widget InvisibleButton
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - WIDTH);

				//Actual Item
				ImGui::InvisibleButton(childEntry.Path.c_str(), ImVec2(WIDTH, HEIGHT), _Renaming ? ImGuiButtonFlags_Disabled : 0);

				//Offset -TEXT_HEIGHT for text
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - TEXT_HEIGHT - 8);

				if (_Renaming && isSelected) {
					ImGui::PushItemWidth(WIDTH);

					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					bool renamingDone = ImGui::InputText("##Rename", _RenamingBuffer.data(), _RenamingBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue);
					ImGui::PopStyleVar(2);

					//First Renaming Frame (Focus)
					if (!_RenamingFocused) {
						ImGui::SetKeyboardFocusHere();
						_RenamingFocused = true;
					}
					else {
						//Renaming Done
						if (renamingDone) {
							SelectItem(_SelectedItem);
						}
						//Cancel Renaming
						else if (!ImGui::IsItemFocused() || ImGui::IsKeyPressed(ImGuiKey_Escape) || !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
							_Renaming = false;
						}
					}

					ImGui::PopItemWidth();
				}
				else {
					ImGui::PushTextWrapPos(offsetX + WIDTH);
					ImGui::TextWrapped(childEntry.Name.c_str());
					ImGui::PopTextWrapPos();
				}
			ImGui::EndGroup();
			EUI::DragAssetSource(childEntry.RelativePath);

			bool isMouseHover = ImGui::IsItemHovered();
			
			//Draw 'Prafab'
			bool isPrefab = childEntry.Name.find(".pf") != std::string::npos;
			if (isPrefab) {
				const char* PREFAB = "Prefab";
				ImVec2 prefabTextSize = ImGui::CalcTextSize(PREFAB);
				ImVec2 textPos = { groupMin.x + (WIDTH - prefabTextSize.x) * 0.5f, groupMin.y + (WIDTH - prefabTextSize.y) * 0.5f };
				ImGui::RenderText(textPos, PREFAB);
			}

			//Offset to cover (Negative Padding)
			groupMin.x -= HOVER_SPACING; groupMin.y -= HOVER_SPACING;
			groupMax.x += HOVER_SPACING; groupMax.y += HOVER_SPACING;

			//Selected
			if (isSelected) {
				ImGui::GetWindowDrawList()->AddRectFilled(groupMin, groupMax, ImColor(255, 255, 255, 50), 4.0f);
			}
			
			//Hover (Select and Context Menu)
			if (isMouseHover && !_Renaming) {
				if (isPrefab) {
					ImGui::SetTooltip("Drag onto scene or double click to open");
				}
				ImGui::GetWindowDrawList()->AddRect(groupMin, groupMax, ImColor(255, 255, 255, 15), 4.0f);

				//Select Item
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)||ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					SelectItem(childEntry.Path.c_str());
				}

				//Run item
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (childEntry.isFolder) {
						_CurrentFolder = childEntry.Path;
					}
					else {
						Editor->OpenAsset(Assets::Hash(childEntry.RelativePath));
					}
				}

				//Context Menu
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					ImGui::OpenPopup(POPUP_ASSET_MENU);
				}

			}

			RenderContextMenu();
			ImGui::PopID();

			offsetX += WIDTH+SPACING;
			sameLine++;
		//}
	});

	//Files
	/*
	_FileExplorer.Explore(entry, [](DirEntry& childEntry) {
		if (!childEntry.isFolder) {

		}
	});
	*/


	//Content Empty Click (Left Clear, Right Context Menu of current folder)
	if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
		SelectItem();
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup(POPUP_ASSET_MENU);
		}
	}


	//Prefab Creation Drag and Drop
	EditorSelection* selection = EUI::DragEntityTarget();
	if (selection) {
		for (auto draggedEntity : selection->GetEntities()) {
			std::string& name = selection->GetWorld()->GetRegistry().get<Hierarchy>(draggedEntity).Name;
			AssetRefT<PrefabAsset> prefab = new PrefabAsset();
			prefab->FromWorld(Editor->Viewport->GetWorld(), draggedEntity);

			Assets::CreateAsset(prefab, (std::filesystem::path(_CurrentFolder).lexically_relative("Mods") / fmt::format("{}.pf", name).c_str()).generic_string());
		}
		_FileExplorer.Invalidate();
	}
	RenderContextMenu();
}

void AssetsWindow::OnEvent(Event& E) {
	if (E.Is<KeyEvent>()) {
		KeyEvent e = E.As<KeyEvent>();
		if (e.Press) {
			if (e.KeyCode == Key::F2) {
				StartRenaming();
			}
			else if (e.KeyCode == Key::Delete) {
				SelectedItemDelete();
			}
		}
	}
}

void AssetsWindow::OnGUI() {

	ImGui::Begin(GetName().c_str());
	GUIInternal();

	_FileExplorer.Refresh();

	RenderHeader();

	ImGui::BeginChild("##DirectoryTree", ImVec2(240, 0), true);
		RenderDirTree(_FileExplorer.GetRoot());
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(10, 10, 10, 100));
		ImGui::BeginChild("##DirectoryContent", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
			RenderDirContent(_FileExplorer.GetDir(_CurrentFolder));
		ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	RenderDeleteDialog();

	ImGui::End();
}

void AssetsWindow::OnUpdate(float dt) {
}
