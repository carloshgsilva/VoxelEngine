#include "Core/Engine.h"
#include "Layer/GameLayer.h"
#include "Editor/EditorLayer.h"

int main() {
	Log::level(Log::L0_Trace);
	Engine::Create();
	//Engine::PushLayer(New<GameLayer>());
	Engine::PushLayer(New<EditorLayer>());
	Engine::Run();
	
	return EXIT_SUCCESS;
}