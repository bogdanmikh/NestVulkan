//
// Created by bogdan on 30.04.2024.
//

#include "Sandbox.hpp"
#include <filesystem>

void Sandbox::start() {
    GlobalSettings settings;
    settings.appName = "Petuh";
    settings.debugMode = true;
    settings.fullScreen = false;
    settings.api = GlobalSettings::Vulkan;
    Application::getInstance()->init(settings);
    Cursor cursor;
    std::string localPath = std::filesystem::current_path().parent_path().parent_path().parent_path().string() + "/";
    std::string fakPath = localPath + "Examples/Sandbox/res/Fak.png";
    std::string figPath = localPath + "Examples/Sandbox/res/Fig.png";
    cursor.update(figPath.c_str());
}

void Sandbox::update(double deltaTime) {
    Cursor cursor;
    std::string localPath = std::filesystem::current_path().parent_path().parent_path().parent_path().string() + "/";
    std::string fakPath = localPath + "Examples/Sandbox/res/Fak.png";
    std::string figPath = localPath + "Examples/Sandbox/res/Fig.png";
    std::string sglazPath = localPath + "Examples/Sandbox/res/Sglaz.png";
    if (Events::isJustKeyPressed(Key::KEY_1)) {
        cursor.update(figPath.c_str());
    } else if (Events::isJustKeyPressed(Key::KEY_2)) {
        cursor.update(fakPath.c_str());
    } else if (Events::isJustKeyPressed(Key::KEY_3)) {
        cursor.update(sglazPath.c_str());
    }
}
