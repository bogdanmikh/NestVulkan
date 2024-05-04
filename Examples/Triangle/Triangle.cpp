//
// Created by bogdan on 30.04.2024.
//

#include "Triangle.hpp"

void Triangle::start() {
    GlobalSettings settings;
    settings.appName = "Petuh";
    settings.fullScreen = true;
    settings.debugMode = true;
    Application::getInstance()->init(settings);
}

void Triangle::update(double deltaTime) {

}
