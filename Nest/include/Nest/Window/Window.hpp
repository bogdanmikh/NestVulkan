#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Window {
public:
    ~Window();
    void init(const char* name, int resolutionX, int resolutionY, bool fullScreen);
    bool shouldClose();
    void setShouldClose();
    void swapBuffers();
    glm::vec2 getSize();
    double getTime();
    void* getNativeHandle();
private:
    void* handle;
};