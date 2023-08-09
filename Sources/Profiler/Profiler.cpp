#pragma once

#include "Profiler.h"
#include "Core/Engine.h"
#include "Profiler/Profiler.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


uint32_t OnImGuiRecursive(RuntimeProfiler::Sample* samples, uint32_t index, uint32_t maxIndex, float scale, float width) {
    while(index < maxIndex) {
        auto& sample = samples[index++];
        bool recurse = index < sample.endIndex;

        float min = float(sample.endTime - sample.beginTime);
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        cursor.x += 14.0f;
        ImVec2 a = {cursor.x - 0.0f, cursor.y + ImGui::GetTextLineHeight()};
        ImVec2 b = {cursor.x + min * scale, a.y + 2.0f};
        ImGui::GetWindowDrawList()->AddRectFilled(a, b, IM_COL32(0, 255, 0, 255));

        bool open = ImGui::TreeNodeEx(sample.name, recurse ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf , "%s", sample.name);

        ImGui::SameLine(width);
        ImGui::Text("%6.2fms", min);

        if(open) {
            index = OnImGuiRecursive(samples, index, sample.endIndex, scale, width);
            ImGui::TreePop();
        } else {
            index = sample.endIndex;
        }
    }
    
    return index;
}

void RuntimeProfiler::OnImGui(float scale, float width) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 4.0f));
    OnImGuiRecursive((RuntimeProfiler::Sample*)GetPrevious().samples, 0, GetPrevious().sampleCount, scale, width);
    ImGui::PopStyleVar();
}

void RuntimeProfiler::MarkFrame() {
    frame = (frame + 1) % RuntimeProfiler::MaxFrameCount;
    frames[frame].sampleCount = 0u;
}

uint32_t RuntimeProfiler::Begin(const char* name, double time) {
    assert(frames[frame].sampleCount < MaxSampleCount);
    frames[frame].samples[frames[frame].sampleCount] = Sample{
        .name = name,
        .beginTime = time,
    };
    return frames[frame].sampleCount++;
}

void RuntimeProfiler::End(uint32_t beginId, double time) {
    assert(frames[frame].sampleCount < MaxSampleCount);
    frames[frame].samples[beginId].endTime = time;
    frames[frame].samples[beginId].endIndex = frames[frame].sampleCount;
}

ProfileCPUScope::ProfileCPUScope(const char* name) {
    SampleID = Engine::GetCPUProfiler().Begin(name, Engine::GetTime());
}

ProfileCPUScope::~ProfileCPUScope() {
    Engine::GetCPUProfiler().End(SampleID, Engine::GetTime());
}