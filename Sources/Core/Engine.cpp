#include "Engine.h"

#include "Input.h"
#include "IO/Log.h"

#include "Job/Jobs.h"
#include "Core/Window.h"
#include "Graphics/Graphics.h"
#include "Profiler/Profiler.h"
#include "Vox/PalleteCache.h"
#include "Mod/ModLoader.h"

#include <GLFW/glfw3.h>

void Engine::StartEngine() {
    while (!Window::Get().ShouldClose()) {
        Update();
    }
    _layerStack.Clear();
}

void Engine::Update() {
    double time = GetTime();
    dt = float(time - previousTime)*1e-3f;
    previousTime = time;

    {
        PROFILE_SCOPE("Update");

        {
            PROFILE_SCOPE("PollEvents");
            glfwPollEvents();
        }

        _OnBeforeUpdate_Callbacks.ExecuteAndClear();

        if (Window::Get().IsFocused()) {
            for (auto& layer : _layerStack) {
                layer->OnUpdate(dt);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    PROFILE_FRAME();
    cpuProfiler.MarkFrame();
    gpuProfiler.MarkFrame();

    if (sleepMS > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMS));
    }
}

void Engine::Create() {
    Engine& e = Get();

    Log::info("[Engine] Initializing...");

    // Initialize EngineSystems
    {
        Jobs::Initialize();
        Window::Initialize();
        Graphics::Initialize();
        PalleteCache::Initialize();
        ModLoader::Initialize();
    }
    // Assets::Initialize()

    Get()._PostInitialize_Callbacks.ExecuteAll();

    Log::info("[Engine] Initialization Done!");
}

void Engine::Run() {
    Get().StartEngine();
}

double Engine::GetTime() {
    return glfwGetTime()*1.0e3;
}
