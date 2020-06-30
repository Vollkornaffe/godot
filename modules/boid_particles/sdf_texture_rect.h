#ifndef SDF_TEXTURE_RECT_H
#define SDF_TEXTURE_RECT_H

#include <limits>

#include "core/object.h"
#include "scene/2d/polygon_2d.h"
#include "scene/gui/texture_rect.h"
#include "util.h"


class SDFTextureRect : public TextureRect {

	GDCLASS(SDFTextureRect, TextureRect);

protected:
	static void _bind_methods() {
		ClassDB::bind_method(
				D_METHOD(
						"calc_from_poly2d", "width", "height", "boundary", "p_poly_2d"),
				&SDFTextureRect::calc_from_poly2d);
	}

public:
	void calc_from_poly2d(
        int width,
        int height,
        float boundary,
        Object *p_object
    ) {

        Polygon2D * p_poly = Object::cast_to<Polygon2D>(p_object);
        if (p_poly == NULL) {
            std::cout << "Cast to Polygon2D failed!" << std::endl;
            return;
        }
        
        const auto v = p_poly->get_polygon();
        const auto N = v.size();

		Ref<Image> p_image;
		p_image.instance();

		p_image->create(width, height, false, Image::FORMAT_RGBF);
		p_image->lock();

        auto size = get_size();
		for (int i = 0; i < width; i++) {

			const auto x = size.x * (float)i / (float)width;

			for (int j = 0; j < height; j++) {

				const auto y = size.y * (float)j / (float)height;

				const auto p = Vector2(x, y);

                Vector2 r;
                float s = 1;
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
                r /= d == 0 ? 1.0 : d;
                r = (r + Vector2(1,1)) / 2.0;
                d /= boundary;
                d = (clamp(d, 0.0, 1.0) * s + 1.0) / 2.0;
                p_image->set_pixel(
                    i, j,
                    Color(r.x, r.y, d));
			}
		}

        Ref<ImageTexture> p_texture;
        p_texture.instance();
		p_texture->create_from_image(p_image);

		set_texture(p_texture);
	}
};

#endif // SDF_TEXTURE_RECT_H