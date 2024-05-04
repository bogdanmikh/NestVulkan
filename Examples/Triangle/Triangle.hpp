#pragma once

#include <Nest.hpp>

class Triangle : public Level {
public:
    void start() override;
    void update(double deltaTime) override;
private:
};
