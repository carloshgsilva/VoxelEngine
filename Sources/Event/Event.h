#pragma once

#include "Core/Core.h"
#include "Core/Input.h"

#include <functional>

// Represents an event that the user may do
// Window -> Engine.DispatchEvent() -> Layer.OnEvent(); -> World.OnEvent();
// EditorLayer may override the ViewportChangeEvent for child World to render inside ImGui Window
struct Event {
	template<typename T> bool Is() { return GetType() == TypeOf<T>(); }
	template<typename T> T& As() { return *static_cast<T*>(this); }
	virtual Type GetType() { return TypeOf<Event>(); }
};

template<typename T>
struct EventDef : public Event {
	virtual Type GetType() { return TypeOf<T>(); }
};

struct ViewportChangeEvent : public EventDef<ViewportChangeEvent> {
	int32 OffsetX, OffsetY, SizeX, SizeY;
};

//////////////////
// Mouse Events //
//////////////////

struct MouseMoveEvent : public EventDef<MouseMoveEvent> {
	float MouseX, MouseY, LastMouseX, LastMouseY;
	float DeltaX, DeltaY;
};

struct MouseButtonEvent : public EventDef<MouseButtonEvent> {
	Button MButton;
	bool Press;
};

//////////////////////
// Keyboards Events //
//////////////////////
struct KeyEvent : public EventDef<KeyEvent> {
	Key KeyCode;
	bool Press;
};

////////////////////
// General Events //
////////////////////
struct DropFileEvent : public EventDef<DropFileEvent> {
	std::vector<std::string> Files;
};
struct FocusChangeEvent : public EventDef<FocusChangeEvent> {
	bool Focused;
};


///////////////////
// Editor Events //
///////////////////
struct AssetImportedEvent : public EventDef<AssetImportedEvent> {
	std::string file;
	std::string toFolder;
};