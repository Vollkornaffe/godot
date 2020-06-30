#ifndef SDF_H
#define SDF_H

#include <iostream>
#include <vector>
#include <limits>

#include "core/reference.h"
#include "scene/2d/polygon_2d.h"

#include "util.h"

class SDF : public Reference {
    GDCLASS(SDF, Reference);

protected:

    static void _bind_methods() {
        ClassDB::bind_method(
            D_METHOD("calc_from_poly2d", "width", "height", "p_poly_2d"),
            &SDF::calc_from_poly2d
        );
        ClassDB::bind_method(
            D_METHOD("get_value", "postion"),
            &SDF::get_value
        );
        ClassDB::bind_method(
            D_METHOD("get_gradient", "postion"),
            &SDF::get_gradient
        );

    }

public:

    inline int idx(int i, int j) const {
        return clamp(i, 0, width - 1) * height
             + clamp(j, 0, height - 1);
    }

    float get_value(Vector2 pos) {
        return values[idx(pos.x, pos.y)];
    }
    Vector2 get_gradient(Vector2 pos) {
        return gradients[idx(pos.x, pos.y)];
    }

	void calc_from_poly2d(
        int _width,
        int _height,
        Object *p_object
    ) {

        width = _width;
        height = _height;

        Polygon2D * p_poly = Object::cast_to<Polygon2D>(p_object);
        if (p_poly == NULL) {
            std::cout << "Cast to Polygon2D failed!" << std::endl;
            return;
        }

        values.reserve(width * height);
        gradients.reserve(width * height);

        const auto v = p_poly->get_polygon();
        const auto N = v.size();

        for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {

				const auto p = Vector2(i, j);

                Vector2 r;
                float s = -1.0;
                //float d = (p-v[0]).length_squared();
                float d = std::numeric_limits<float>::max();
                int l = N-1;
                for (int k = 0; k < N; k++) {

                    // distance
                    auto e = v[l] - v[k];
                    auto w = p - v[k];
                    auto b = w - e * clamp(w.dot(e) / e.length_squared(), 0.0, 1.0);
                    auto b_ls = b.length_squared();

                    if (d > b_ls) {
                        d = b_ls;
                        r = b;
                    }
                    if ((p.y>=v[k].y && p.y<v[l].y && e.x*w.y>e.y*w.x)
                    ||  (p.y<v[k].y && p.y>=v[l].y && e.x*w.y<=e.y*w.x)
                    ) {
                        s *= -1.0;
                    } 

                    l = k;

                }

                d = r.length();
                values.push_back(d * s);
                gradients.push_back(s * r/(d == 0 ? 1.0 : d));
			}
		}
	}

    int width;
    int height;
    std::vector<float> values;
    std::vector<Vector2> gradients;

};

#endif // SDF_H