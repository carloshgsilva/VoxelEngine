#include "Input.h"

#include "Window.h"
#include <windows.h>
#include <GLFW/glfw3.h>

bool Input::IsKeyPressed(Key key) {
    GLFWwindow* w = Window::Get().GetNativeWindow();
    return glfwGetKey(w, static_cast<int>(key)) == GLFW_PRESS;
}

bool Input::IsKeyDown(Key key) {
    GLFWwindow* w = Window::Get().GetNativeWindow();
    return glfwGetKey(w, static_cast<int>(key)) != GLFW_RELEASE;
}

bool Input::IsButtonPressed(Button button) {
    GLFWwindow* w = Window::Get().GetNativeWindow();
    return  glfwGetMouseButton(w, static_cast<int>(button)) == GLFW_PRESS;
}
