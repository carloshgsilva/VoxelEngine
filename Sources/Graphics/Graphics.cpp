#include "Graphics.h"

#include <evk/evk_vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


#include "IO/Log.h"
#include "Core/Engine.h"

#include "Profiler/Profiler.h"

Graphics::Graphics() {
    GLFWwindow* window = Window::Get().GetNativeWindow();

    EvkDesc evkInitDesc = {};
    // Add GLFW instanceExtensions
    {
        uint32_t count = 0;
        const char** instanceExtensions = glfwGetRequiredInstanceExtensions(&count);
        for (uint32_t i = 0; i < count; i++) {
            evkInitDesc.instanceExtensions.push_back(instanceExtensions[i]);
        }
    }

    CHECK(evk::InitializeEVK(evkInitDesc));

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(evk::GetState().instance, window, nullptr, &surface) != VK_SUCCESS) {
        Log::error("Failed to create window surface!");
    }
    if (!InitializeSwapchain(surface)) {
        Log::error("Failed to initialize swapchain!");
    }
}