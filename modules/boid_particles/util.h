#pragma once

#include <cassert>
#include "core/math/vector2.h"
#include "core/math/random_number_generator.h"
#include "core/math/rect2.h"

constexpr const real_t& clamp( const real_t& v, const real_t& lo, const real_t& hi )
{
    assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

inline Vector2 clamp(Vector2 const& x, Vector2 const& min, Vector2 const& max) {

    return Vector2(
        x.x < min.x ? min.x : x.x > max.x ? max.x : x.x,
        x.y < min.y ? min.y : x.y > max.y ? max.y : x.y
    );

}

struct RandomCustom : public RandomNumberGenerator {

    inline Vector2 rand_within_rect(Rect2 const& rect) {

        auto min = rect.position;
        auto max = min + rect.size;

        return Vector2(

            randf_range(min.x, max.x),
            randf_range(min.y, max.y)

        );

    }

};