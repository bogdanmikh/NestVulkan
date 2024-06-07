//
// Created by bogdan on 20.05.2024.
//
#include "Nest/Objects/Cursor.hpp"
#include "Nest/Application/Application.hpp"

#include <GLFW/glfw3.h>
#include <stb_image.h>

void Cursor::update(const char *path) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = data;

    GLFWcursor* cursor = glfwCreateCursor(&image, width / 2, height / 2);
    auto window = static_cast<GLFWwindow*>(Application::getInstance()->getWindow()->getNativeHandle());
    glfwSetCursor(window, cursor);
}

