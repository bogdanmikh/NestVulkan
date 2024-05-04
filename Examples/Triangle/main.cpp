#include <iostream>

#include <Nest.hpp>

#include "Triangle.hpp"

 Application* Application::s_instance = new Application;

int main() {
    auto *app = Application::getInstance();
    auto *level = new Triangle();
    app->setLevel(level);
    level->start();
    app->loop();
    delete level;
    delete app;
}