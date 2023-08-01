#pragma once

#include "Core.h"
#include "LayerStack.h"
#include "Util/CallbackQueue.h"
#include "Profiler/Profiler.h"

class Engine {
    RuntimeProfiler profiler = {};
    CallbackQueue<void> _PostInitialize_Callbacks;
    CallbackQueue<Event&> _OnEvent_Callbacks;
    CallbackQueue<void> _OnBeforeUpdate_Callbacks;
    LayerStack _layerStack;
    float dt = 0.0f; // in seconds per frame
    double previousTime = 0.0f;

    void StartEngine();
    void Update();

   public:
    int sleepMS = 0;

    void DispatchEvent(Event& E) {
        _OnEvent_Callbacks.ExecuteAll(E);
        if (E.Is<ViewportChangeEvent>()) {
            Update();
        }
        for (auto& L : _layerStack) {
            L->OnEvent(E);
        }
    }

    static Engine& Get() {
        static Engine engine;
        return engine;
    }

    static void Create();

    static void PushLayer(Ref<Layer> layer) {
        Get()._layerStack.PushLayer(layer);
    }
    static void PopLayer() {
    }
    static void PushOverlay() {
    }
    static void PopOverlay() {
    }

    static RuntimeProfiler& GetRuntimeProfiler() { return Get().profiler; }

    static void Run();

    static void OnBeforeUpdate(std::function<void()> _Func) {
        Get()._OnBeforeUpdate_Callbacks.AddCallback(_Func);
    }

    // Used by EngineSystems to Add An Callback that is called after all Systems have been initialized
    static void Bind_PostInitialize(std::function<void()> _Func) {
        Get()._PostInitialize_Callbacks.AddCallback(_Func);
    }
    // Used by EngineSystems to Handle Events
    static void Bind_OnEvent(std::function<void(Event&)> _Func) {
        Get()._OnEvent_Callbacks.AddCallback(_Func);
    }

    static double GetTime();
    inline static float GetDt() {
        return Get().dt;
    }
};