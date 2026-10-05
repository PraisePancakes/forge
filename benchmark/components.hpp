#pragma once
#include <iostream>
namespace benchmark {
struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

struct Health {
    int value;
};

struct Name {
    std::string value;
};
}  // namespace benchmark