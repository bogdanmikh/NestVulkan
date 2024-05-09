#pragma once

#include <string>

#include "Nest/Window/Window.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Objects/Level.hpp"
#include "Nest/Renderer/Renderer.hpp"

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

class Application {
public:
    ~Application();
    inline static Application* getInstance() {
        return s_instance;
    }
    void init(const GlobalSettings& globalSettings);
    void loop();
    void close();

    inline Window* getWindow() {
        return window;
    }

    inline int getMaxFps() const {
        return maximumFps;
    }

    inline int getFps() const {
        return fps;
    }

    inline void setLevel(Level* level) {
        currentLevel = level;
    }
private:
    Application();
    static Application *s_instance;
    Window* window;
    Level *currentLevel;
    Renderer *renderer;

    bool debugMode;

    int fps;
    int maximumFps = 60;
    uint64_t deltaTimeMillis = 0;
    int thisSecondFramesCount = 0;
    uint64_t timeMillis;
    uint64_t oneSecondTimeCount = 0;
};