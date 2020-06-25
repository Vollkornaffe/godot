#pragma once

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
};