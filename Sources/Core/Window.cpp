#include "Window.h"

#include "IO/Log.h"
#include "Engine.h"

#include <GLFW/glfw3.h>

Window::Window() {
    title = "Engine";
    width = 640;
    height = 480;

    // Init GLFW
    if (!glfwInit()) {
        Log::error("Failed to Init GLFW!");
        exit(-1);
    }

    // Init Vulkan
    if (!glfwVulkanSupported()) {
        throw std::exception("Vulkan not supported!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window) {
        Log::error("Failed to open Window!");
        exit(-1);
    }

    glfwSetWindowUserPointer(window, this);

    // TODO: Do All The Callbacks
    // There should be an event dispatcher in the Engine class
    // Callbacks
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int newWidth, int newHeight) {
        Window* W = (Window*)glfwGetWindowUserPointer(window);
        W->SetSize(newWidth, newHeight);

        ViewportChangeEvent E;
        E.OffsetX = 0;
        E.OffsetY = 0;
        E.SizeX = newWidth;
        E.SizeY = newHeight;
        Engine::Get().DispatchEvent(E);
    });
    glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {});
    glfwSetDropCallback(window, [](GLFWwindow* window, int count, const char** filesName) {
        Window* W = (Window*)glfwGetWindowUserPointer(window);

        DropFileEvent E;
        for (int i = 0; i < count; i++) {
            E.Files.push_back(filesName[i]);
        }

        Engine::Get().DispatchEvent(E);
    });
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Window* W = (Window*)glfwGetWindowUserPointer(window);

        KeyEvent E;
        E.KeyCode = (Key)key;
        E.Press = action != GLFW_RELEASE;
        Engine::Get().DispatchEvent(E);
    });
    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int keycode) {});
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        Window* W = (Window*)glfwGetWindowUserPointer(window);

        MouseButtonEvent E;
        E.MButton = (Button)button;
        E.Press = action != GLFW_RELEASE;
        Engine::Get().DispatchEvent(E);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {});
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos) {
        Window* W = (Window*)glfwGetWindowUserPointer(window);

        MouseMoveEvent E;
        E.MouseX = xPos;
        E.MouseY = yPos;
        E.LastMouseX = W->lastMouseX;
        E.LastMouseY = W->lastMouseY;
        E.DeltaX = xPos - W->lastMouseX;
        E.DeltaY = yPos - W->lastMouseY;
        Engine::Get().DispatchEvent(E);

        W->lastMouseX = xPos;
        W->lastMouseY = yPos;
    });
    glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused) {
        Window* W = (Window*)glfwGetWindowUserPointer(window);
        W->isFocused = focused;
    });
}

Window::~Window() {
    glfwDestroyWindow(window);
}

bool Window::IsFocused() {
    return isFocused;
}

bool Window::IsFullscreen() {
    return glfwGetWindowMonitor(GetNativeWindow()) != nullptr;
}
void Window::FullscreenToggle() {
    return SetFullscreen(!IsFullscreen());
}

void Window::SetFullscreen(bool Fullscreen) {
    if (Fullscreen) {
        int monitorsCount = 0;
        GLFWmonitor* monitor = glfwGetMonitors(&monitorsCount)[0];
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(Window::Get().GetNativeWindow(), monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(Window::Get().GetNativeWindow(), NULL, 300, 200, 600, 400, GLFW_DONT_CARE);
    }
}

GLFWwindow* Window::GetNativeWindow() const {
    return window;
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::SetSize(int32 Width, int32 Height) {
    this->width = Width;
    this->height = Height;
}

int32 Window::GetWidth() const {
    return width;
}

int32 Window::GetHeight() const {
    return height;
}
