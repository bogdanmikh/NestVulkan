#pragma once

#include "Nest/Objects/GlobalSettings.hpp"

struct Renderer {
    virtual void init(const GlobalSettings &globalSettings) = 0;
    virtual void render() = 0;
    virtual ~Renderer() = default;
};