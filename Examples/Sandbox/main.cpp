#include <iostream>

#include <Nest.hpp>

#include "Sandbox.hpp"

 Application* Application::s_instance = new Application;

int main() {
    auto *app = Application::getInstance();
    auto *level = new Sandbox();
    app->setLevel(level);
    level->start();
    app->loop();
    delete level;
    delete app;
}