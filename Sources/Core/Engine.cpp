#include "Engine.h"

#include "Input.h"
#include "DeltaTime.h"
#include "IO/Log.h"

#include "Job/Jobs.h"
#include "Core/Window.h"
#include "Graphics/Graphics.h"
#include "Profiler/Profiler.h"
#include "Vox/PalleteCache.h"
#include "Mod/ModLoader.h"

#include <GLFW/glfw3.h>


void Engine::StartEngine()
{
	Profiler::Begin("Tracing.json");
	while (!Window::Get().ShouldClose()) {
		Update();
	}
	_layerStack.Clear();
	Profiler::End();
}

float elapsedFrameTime = 0.0f;
float elapsedFrameCount = 0.0f;
void Engine::Update() {
	PROFILE_FUNC()

	float time = (float)glfwGetTime();
	DeltaTime dt = time - _lastTime;
	_lastTime = time;

	{
		PROFILE_SCOPE("glfwPollEvents")
		glfwPollEvents();
	}


	//Run OnBeforeUpdate
	_OnBeforeUpdate_Callbacks.ExecuteAndClear();

	if (elapsedFrameTime >= 1.0f) {
		float t = elapsedFrameTime / elapsedFrameCount;
		//Log::info("Engine::Tick() {}ms {}fps", t * 1000.0, 1.0f / t);
		elapsedFrameTime = 0;
		elapsedFrameCount = 0;
	}
	elapsedFrameCount++;
	elapsedFrameTime += dt;

	for (auto& layer : _layerStack) {
		layer->OnUpdate(dt);
	}

	Window::Get().Swap();
}


void Engine::Create()
{
	Engine& e = Get();

	Log::info("[Engine] Initializing...");

	//Initialize EngineSystems
	{
		PreciseTimer t("[Engine] Init Systems");

		Jobs::Initialize();
		Window::Initialize();
		Window::Get().SetVSync(true);
		Graphics::Initialize();
		PalleteCache::Initialize();
		Profiler::Initialize();
		ModLoader::Initialize();
	}
	//Assets::Initialize()

	Get()._PostInitialize_Callbacks.ExecuteAll();

	Log::info("[Engine] Initialization Done!");
}

void Engine::Run() {
	Get().StartEngine();
}

double Engine::GetTime()
{
	return glfwGetTime();
}
