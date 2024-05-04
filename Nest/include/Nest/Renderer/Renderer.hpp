#pragma once

struct Renderer {
    virtual void init(bool debug, const char* appName) = 0;
};