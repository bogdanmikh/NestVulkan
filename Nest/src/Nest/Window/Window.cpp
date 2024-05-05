#include <cstring>

#include "Nest/Window/Window.hpp"
#include "Nest/Logger/Logger.hpp"

void Window::init(const char *name, int resolutionX, int resolutionY, bool fullScreen) {
    if (glfwInit() != GLFW_TRUE) {
        LOG_ERROR("GLFW initialization failed\n");
        return;
    }

    if (fullScreen) glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
#if defined(__APPLE__) || defined(__MACH__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    GLFWwindow *window = glfwCreateWindow(resolutionX, resolutionY, name, NULL, NULL);
    if (!window) {
        LOG_ERROR("GLFW window creation failed\n");
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    this->handle = window;
}

Window::~Window() {
    glfwTerminate();
}

bool Window::shouldClose() {
    return glfwWindowShouldClose((GLFWwindow*) handle);
}

glm::vec2 Window::getSize() {
    int x, y;
    float xscale, yscale;
    glfwGetWindowContentScale((GLFWwindow*) handle, &xscale, &yscale);
    glfwGetWindowSize((GLFWwindow*) handle,  &x, &y);
    return { x * xscale, y * yscale };
}

double Window::getTime() {
    return glfwGetTime();
}

void Window::swapBuffers() {
    glfwSwapBuffers((GLFWwindow*) handle);
}

void Window::setShouldClose() {
    glfwSetWindowShouldClose((GLFWwindow*) handle, true);
}

void* Window::getNativeHandle() {
    return handle;
}
