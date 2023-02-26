#pragma once

#include "Core.h"
#include "Module.h"

#include <string>

struct GLFWwindow;

class Window : public ModuleDef<Window> {
    GLFWwindow* window;
    std::string title;
    int32 width;
    int32 height;

    bool isFocused = true;
    float lastMouseX;
    float lastMouseY;

   public:
    Window();
    ~Window();

    bool IsFocused();
    bool IsFullscreen();
    void FullscreenToggle();
    void SetFullscreen(bool Fullscreen);

    bool ShouldClose() const;
    void SetSize(int32 Width, int32 Height);
    int32 GetWidth() const;
    int32 GetHeight() const;
    GLFWwindow* GetNativeWindow() const;
};