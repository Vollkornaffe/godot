#pragma once

#include "core/math/vector2.h"

inline Vector2 clamp(Vector2 const& x, Vector2 const& min, Vector2 const& max) {

    return Vector2(
        x.x < min.x ? min.x : x.x > max.x ? max.x : x.x,
        x.y < min.y ? min.y : x.y > max.y ? max.y : x.y
    );

}