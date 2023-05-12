#pragma once

#include "Profiler/Profiler.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Profiler.h"
#include "Core/Engine.h"

void RuntimeProfiler::Measure(const char* name, float time) {
    auto& ts = timestamps[name];
    ts.window[ts.index] = time;
    ts.index = (ts.index + 1) % MAX_WINDOW_SIZE;
}

void RuntimeProfiler::OnImGui(float width) const {
    constexpr float SCALE = 200.0f;
    for (auto& ts : timestamps) {
        float min = FLT_MAX;
        float max = -FLT_MAX;
        for(int i = 0; i < MAX_WINDOW_SIZE; i++) {
            float t = ts.second.window[i];
            if(t == 0.0f) continue;
            min = std::min(min, t);
            max = std::max(max, t);
        }

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        cursor.x += width + 30.0f;
        ImVec2 a = {cursor.x, cursor.y};
        ImVec2 b = {cursor.x + min * SCALE, a.y + ImGui::GetFontSize()};
        ImGui::GetWindowDrawList()->AddRectFilled(a, b, IM_COL32(0, 255, 0, 255));
        a = {cursor.x + min * SCALE - 0.5f, cursor.y + ImGui::GetFontSize() * 0.5f};
        b = {a.x + (max-min)*SCALE + 0.5f, a.y};
        ImGui::GetWindowDrawList()->AddLine(a, b, IM_COL32(9, 255, 0, 255));
        a = {cursor.x + max * SCALE, cursor.y};
        b = {a.x, a.y + ImGui::GetFontSize()};
        ImGui::GetWindowDrawList()->AddLine(a, b, IM_COL32(9, 255, 0, 255));
        ImGui::Text("%s", ts.first);
        ImGui::SameLine(width);
        ImGui::Text("%6.2f", min);
        if(ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PlotLines("", ts.second.window, MAX_WINDOW_SIZE, ts.second.index);
            ImGui::EndTooltip();
        }

    }
}

ProfileScope::ProfileScope(const char* name) : name(name) {
    begin = Engine::GetTime();
    entryIndex = Profiler::GetCurrentCapture().Begin(name, begin);
}
ProfileScope::~ProfileScope() {
    double end = Engine::GetTime();
    Profiler::GetCurrentCapture().End(entryIndex, end);
    Engine::GetRuntimeProfiler().Measure(name, float(end - begin));
}
