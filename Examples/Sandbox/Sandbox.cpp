//
// Created by bogdan on 30.04.2024.
//

#include "Sandbox.hpp"

void Sandbox::start() {
    GlobalSettings settings;
    settings.appName = "Petuh";
    settings.debugMode = true;
    settings.api = GlobalSettings::Vulkan;
    Application::getInstance()->init(settings);
}

void Sandbox::update(double deltaTime) {

}
