#include "Graphics.h"

#define EVK_INTERNAL_STATE
#include <evk/evk.h>
#include <GLFW/glfw3.h>
#include "IO/Log.h"
#include "Core/Engine.h"
#include "Util.h"

#include "Profiler/Profiler.h"

Graphics::Graphics() {

	GLFWwindow* window = Window::Get().GetNativeWindow();

	auto initInfo = evk::InitInfo();
	//Add GLFW instanceExtensions
	{
		uint32_t count = 0;
		const char** instanceExtensions = glfwGetRequiredInstanceExtensions(&count);
		for (int i = 0; i < count; i++) {
			initInfo.addInstanceExtension(instanceExtensions[i]);
		}
	}

#ifdef _DEBUG
	initInfo.addInstanceExtension("VK_EXT_debug_report");
	initInfo.addInstanceLayer("VK_LAYER_KHRONOS_validation");
#endif

	if (!evk::InitializeEVK(initInfo)) {
		Log::critical("Failed to init evk!");
	}

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(evk::GetState().instance, window, nullptr, &surface) != VK_SUCCESS) {
		Log::critical("Failed to create window surface!");
	}

	if (!InitializeSwapchain(surface)) {
		Log::critical("Failed to initialize swapchain!");
	}

	frameCmdBuffer = CmdBuffer::Create();
	transferCmdBuffer = CmdBuffer::Create();
}
