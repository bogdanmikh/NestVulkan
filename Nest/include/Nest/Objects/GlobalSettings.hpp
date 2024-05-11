#pragma once

#include <string>

struct GlobalSettings {
    enum GraphicsAPI {
        OpenGL,
        Vulkan
    };
    GlobalSettings()
            : appName("GLFW Window")
            , resolutionX(800)
            , resolutionY(600)
            , fullScreen(false)
            , debugMode(true)
            , api(Vulkan) {}
    std::string appName;
    GraphicsAPI api;
    int resolutionX;
    int resolutionY;
    bool fullScreen;
    bool debugMode;
};