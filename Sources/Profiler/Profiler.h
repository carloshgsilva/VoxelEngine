#pragma once

#include "Core/Module.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <mutex>
#include <map>

#define ACTIVATE_PROFILER 1

struct FrameScopeCapture {
    static constexpr uint32 MAX_ENTRY_COUNT = 1024;
    struct Entry {
        double begin;
        double end;
        const char* name;
    };

    uint32 Begin(const char* name, double begin) {
        uint32 idx = count++;
        entries[idx] = {begin, 0.0, name};
        return idx;
    }
    void End(uint32 index, double end) {
        entries[index].end = end;
    }
    void Clear() {
        count = 0;
    }

    template<typename BeginT, typename EndT>
    void Iterate(const BeginT& begin, const EndT& end) {
        uint32 stack[64];
        int stack_id = -1;

        for(int i = 0; i < count; i++) {
            Entry& entry = entries[i];
            while(stack_id >= 0 && (entries[stack[stack_id]].end < entry.begin)) {
                Entry& stack_entry = entries[stack[stack_id]];
                end();
                stack_id--;
            }

            stack[++stack_id] = i;
            begin(entry);
        }

        while(stack_id >= 0) {
            Entry& stack_entry = entries[stack[stack_id]];
            end();
            stack_id--;
        }
    }

    int count = 0;
    Entry entries[MAX_ENTRY_COUNT];
};

class Profiler : public ModuleDef<Profiler> {
    static constexpr uint32 MAX_CAPTURE_COUNT = 32;
public:
    static void AdvanceFrame() {
        Get().index = (Get().index + 1) % MAX_CAPTURE_COUNT;
    }

    static FrameScopeCapture& GetCurrentCapture() {
        return Get().framesCapture[Get().index % MAX_CAPTURE_COUNT];
    }
    static FrameScopeCapture& GetPreviousCapture() {
        return Get().framesCapture[(Get().index + MAX_CAPTURE_COUNT - 1) % MAX_CAPTURE_COUNT];
    }

    uint32 index = 0;
    FrameScopeCapture framesCapture[MAX_CAPTURE_COUNT];
};

struct ProfileScope {
    ProfileScope(const char* name);
    ~ProfileScope();
private:
    double begin = 0.0f;
    const char* name;
    uint32 entryIndex = 0;
};

#if ACTIVATE_PROFILER
    #define PROFILE_FUNC() ProfileScope __profile##__LINE__(__FUNCTION__);
    #define PROFILE_SCOPE(name) ProfileScope __profile##__LINE__(name);
#else
    #define PROFILE_FUNC()
    #define PROFILE_SCOPE(name)
#endif

struct RuntimeProfiler {
    static constexpr uint32 MAX_WINDOW_SIZE = 32;
    struct Window {
        float window[MAX_WINDOW_SIZE] = {};
        uint32 index = 0;
    };

    void Measure(const char* name, float time);
    void OnImGui(float width) const;

private:
    std::map<const char*, Window> timestamps = {};
};