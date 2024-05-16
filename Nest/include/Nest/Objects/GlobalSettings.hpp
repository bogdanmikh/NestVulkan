#pragma once

#include <string>

struct GlobalSettings {
    enum GraphicsAPI {
        OpenGL,
        Vulkan
    };

    GlobalSettings()
            : appName("GLFW Window"), resolutionX(640), resolutionY(480), fullScreen(false), debugMode(true),
              api(Vulkan) {}

    std::string appName;
    GraphicsAPI api;
    int resolutionX;
    int resolutionY;
    bool fullScreen;
    bool debugMode;
};