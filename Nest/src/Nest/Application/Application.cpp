#include <chrono>

#include "Nest/Application/Application.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Vulkan.hpp"

using namespace vk;

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;

Application::Application() {
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

Application::~Application() {
    delete window;
}

void Application::init(const GlobalSettings& globalSettings) {
    window = new Window;
    window->init(globalSettings.appName.c_str(), globalSettings.resolutionX,
                 globalSettings.resolutionY, globalSettings.fullScreen);
    if (globalSettings.api == GlobalSettings::Vulkan) {
        renderer = new Vulkan;
        renderer->init(globalSettings.debugMode, globalSettings.appName.c_str());
    } else if (globalSettings.api == GlobalSettings::OpenGL) {
        LOG_WARN("OpenGL not supported now");
    }
}

uint64_t getMillis() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    return now.count();
}

void Application::loop() {
    if (!window) {
        LOG_ERROR("Window NOT INIT!");
        return;
    }
    if (!currentLevel) {
        LOG_ERROR("Level IS NULL");
        return;
    }
    if (!renderer) {
        LOG_WARN("Renderer not init!");
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
            LOG_INFO("FPS: {}", fps);
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