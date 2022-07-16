#pragma once

#include "EditorWindow.h"
#include "IO/FileExplorer.h"

#include "imgui/imgui.h"
#include "imgui/IconsFontAwesome5.h"

class AssetsWindow : public EditorWindow {
	std::string _CurrentFolder;
	std::string _SelectedItem;
	FileExplorer _FileExplorer;

	char _Filter[64] = { '\0' };
	bool _DeletionPending = false, _DeletionBegin = false, _Renaming = false, _RenamingFocused = false;
	std::string _RenamingBuffer;

	inline static const char* POPUP_ASSET_MENU = "Asset Context Menu";
	inline static const char* POPUP_DIR_MENU = "Directory Context Menu";
	inline static const char* POPUP_DELETE = "Are you sure you want to delete?";

public:
	AssetsWindow();

	void SelectedItemClone() {
		std::filesystem::path srcItem = std::filesystem::current_path() / GetTargetPath();
		std::filesystem::path dstItem = srcItem.parent_path() / srcItem.filename().concat(" (Clone)");

		std::filesystem::copy(srcItem, dstItem);
		_FileExplorer.Invalidate();
	}
	void SelectedItemDelete(){
		if (_SelectedItem.empty())return;
		_DeletionPending = true;
		_DeletionBegin = true;
	}

	void StartRenaming() {
		_Renaming = true;
		_RenamingFocused = false;
		_RenamingBuffer = GetTargetPath().filename().string();
		_RenamingBuffer.resize(256);
	}

	void SelectItem(std::string item = "") {

		if (_Renaming) {
			_Renaming = false;
			std::filesystem::path newPath = GetTargetPath().make_preferred().parent_path() / _RenamingBuffer;
			std::filesystem::rename(GetTargetPath(), newPath);
			_FileExplorer.Invalidate();
			Log::debug("Renamed from {} to {}", _SelectedItem, newPath.string());
		}
		_SelectedItem = item;
	}

	std::filesystem::path GetTargetPath() { return std::filesystem::path(_SelectedItem.empty() ? _CurrentFolder : _SelectedItem); };
	std::string GetCurrentFolder();
	void Refresh() { _FileExplorer.Invalidate(); }

	void RenderFolderIcon(ImVec2 min, ImVec2 max);
	void RenderDeleteDialog();
	void RenderContextMenu();
	void RenderHeader();
	void RenderDirTree(DirEntry& entry);
	void RenderDirContent(DirEntry& entry);

	virtual void OnEvent(Event& E);
	virtual void OnGUI();
	virtual void OnUpdate(float dt);

	virtual std::string GetName() { return ICON_FA_DESKTOP "  Assets"; }
};