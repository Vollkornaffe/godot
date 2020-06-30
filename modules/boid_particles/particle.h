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
    float rotation;
    Vector2 velocity;
    bool active;
    float angle_rand;
    float scale_rand;
    float hue_rot_rand;
    float anim_offset_rand;
    float time;
    float lifetime;
    Color base_color;

    uint32_t seed;


public:

    void print() {

        std::cout << "Transform2D transform; " << transform << std::endl;
        std::cout << "Color color; " << color << std::endl;
        std::cout << "float custom[4]; " << custom[0] << ", " << custom[1] << ", " << custom[2] << ", " << custom[3] <<  std::endl;
        std::cout << "float rotation; " << rotation << std::endl;
        std::cout << "Vector2 velocity; " << velocity << std::endl;
        std::cout << "bool active; " << active << std::endl;
        std::cout << "float angle_rand; " << angle_rand << std::endl;
        std::cout << "float scale_rand; " << scale_rand << std::endl;
        std::cout << "float hue_rot_rand; " << hue_rot_rand << std::endl;
        std::cout << "float anim_offset_rand; " << anim_offset_rand << std::endl;
        std::cout << "float time; " << time << std::endl;
        std::cout << "float lifetime; " << lifetime << std::endl;
        std::cout << "Color base_color; " << base_color << std::endl;
        std::cout << "uint32_t seed; " << seed << std::endl;

    }
};