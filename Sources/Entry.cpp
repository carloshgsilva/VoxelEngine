#include "Core/Engine.h"
#include "Layer/GameLayer.h"
#include "Editor/EditorLayer.h"

int main() {
	Engine::Create();
	//Engine::PushLayer(New<GameLayer>());
	Engine::PushLayer(New<EditorLayer>());
	Engine::Run();
	
	return EXIT_SUCCESS;
}