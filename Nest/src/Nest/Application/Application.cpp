#include <chrono>
#include <cassert>

#include "Nest/Application/Application.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Vulkan.hpp"

using namespace vk;

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;

Application::Application() : debugMode(true) {}

Application::~Application() {
    delete window;
    delete renderer;
}

void Application::init(const GlobalSettings& globalSettings) {
    debugMode = globalSettings.debugMode;
    if (debugMode) {
        Logger::init();
        LOG_INFO("Application Start!");
        std::string message = {"\n       |-- \\\n"
                               "       |     \\\n"
                               "       |     /\n"
                               "       |-- /\n"
                               "       |\n"
                               "  \\    |    /\n"
                               "    \\  |  /\n"
                               " α    \\|/    Ω\n"
                               "      /|\\\n"
                               "    /  |  \\\n"
                               "  /    |    \\\n"
                               "   HOC VINCE"};
        LOG_INFO(message);
    }
    window = new Window;
    window->init(globalSettings.appName.c_str(), globalSettings.resolutionX,
                 globalSettings.resolutionY, globalSettings.fullScreen);
    if (globalSettings.api == GlobalSettings::Vulkan) {
        if (!glfwVulkanSupported()) {
            if (debugMode) {
                LOG_ERROR("GLFW NOT SUPPORT VULKAN!");
            }
            return;
        }
        renderer = new Vulkan;
        renderer->init(debugMode, globalSettings.appName.c_str());
    } else if (globalSettings.api == GlobalSettings::OpenGL) {
        LOG_ERROR("OpenGL not supported now");
    }
}

uint64_t getMillis() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    return now.count();
}

void Application::loop() {
    if (!window) {
        if (debugMode) {
            LOG_ERROR("Window NOT INIT!");
        }
        return;
    }
    if (!currentLevel) {
        if (debugMode) {
            LOG_ERROR("Level IS NULL");
        }
        return;
    }
    if (!renderer) {
        if (debugMode) {
            LOG_ERROR("Renderer not init!");
        }
        return;
    }

    while (!window->shouldClose()) {
        uint64_t lastTime = timeMillis;
        timeMillis = getMillis();
        deltaTimeMillis += timeMillis - lastTime;
        if (deltaTimeMillis < (1000 / maximumFps)) {
            continue;
        }
        oneSecondTimeCount += deltaTimeMillis;

        thisSecondFramesCount++;
        if (oneSecondTimeCount >= 1000) {
            fps = thisSecondFramesCount;
            if (debugMode) {
                LOG_INFO("FPS: {}", fps);
            }
            thisSecondFramesCount = 0;
            oneSecondTimeCount = 0;
        }

        double deltaTime = deltaTimeMillis / 1000.0;
        deltaTimeMillis = 0;

        currentLevel->update(deltaTime);
        window->swapBuffers();
        glfwPollEvents();
    }
}

void Application::close() {
    window->setShouldClose();
}