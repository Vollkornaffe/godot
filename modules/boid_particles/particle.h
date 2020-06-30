#pragma once

#include <iostream>
#include "util.h"

#include "core/math/transform_2d.h"
#include "core/color.h"

// warning - beware of adding non-trivial types
// to this structure as it is zeroed to initialize in set_amount()
struct Particle {
    Transform2D transform;
    Color color;
    float custom[4];
    Vector2 velocity;
    bool active;
    float time;
    uint32_t seed;


public:

    void print() const {

        std::cout << "Transform2D transform; " << transform << std::endl;
        std::cout << "Color color; " << color << std::endl;
        std::cout << "float custom[4]; " << custom[0] << ", " << custom[1] << ", " << custom[2] << ", " << custom[3] <<  std::endl;
        std::cout << "Vector2 velocity; " << velocity << std::endl;
        std::cout << "bool active; " << active << std::endl;
        std::cout << "float time; " << time << std::endl;
        std::cout << "uint32_t seed; " << seed << std::endl;

    }
};