#include "util.h"

std::ostream& operator<<(std::ostream& os, const Vector2& v) {
    os << "(" << v.x << "," << v.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Transform2D& t) {
    os << "Transform2D(" << t[0] << ", " << t[1] << ", " << t[2] << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Color& c) {
    os << "Color(" << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3] << ")";
    return os;
}

