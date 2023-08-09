#pragma once

#include "Core/Module.h"

#define ACTIVATE_PROFILER 1

#define TRACY_ENABLE 1
#include <tracy/Tracy.hpp>

struct RuntimeProfiler {
    static constexpr uint32 MaxFrameCount = 2;
    static constexpr uint32 MaxSampleCount = 4096;

    struct Sample {
        const char* name;
        double beginTime;
        double endTime;
        uint32_t endIndex;
    };

    struct Frame {
        uint64_t frame;
        Sample samples[MaxSampleCount];
        uint32_t sampleCount;
    };

    uint32_t Begin(const char* name, double time);
    void End(uint32_t beginId, double time);
    void OnImGui(float scale, float width);
    void MarkFrame();

    Frame& GetCurrent() { return frames[frame]; }
    Frame& GetPrevious() { return frames[(frame + MaxFrameCount - 1u) % MaxFrameCount]; }

    uint32_t frame = 0u;
    Frame frames[MaxFrameCount] = {};
};

struct ProfileCPUScope {
    ProfileCPUScope(const char* name);
    ~ProfileCPUScope();
private:
    uint32_t SampleID;
};


#if ACTIVATE_PROFILER
    #define PROFILE_FUNC() ProfileCPUScope __profile_##__LINE__(__FUNCTION__); ZoneScoped
    #define PROFILE_SCOPE(name) ProfileCPUScope __profile_scope_##__LINE__(name); ZoneScopedN(name)
    #define PROFILE_FRAME() FrameMark
#else
    #define PROFILE_FUNC()
    #define PROFILE_SCOPE(name)
    #define PROFILE_FRAME()
#endif
