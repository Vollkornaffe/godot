/*************************************************************************/
/*  cpu_particles_2d.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef BOID_PARTICLES_H
#define BOID_PARTICLES_H

#include <vector>

#include "core/rid.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"
#include "util.h"

class BoidParticles : public Node2D {
private:
	GDCLASS(BoidParticles, Node2D);

public:

	enum Parameter {

		PARAM_DETECTION_RANGE,
		PARAM_DETECTION_ANGLE,
		PARAM_REPULSION,
		PARAM_CLUMPING,
		PARAM_ALIGNING,
		PARAM_DAMPING,
		PARAM_SCALE,
		PARAM_ANIM_SPEED,
		PARAM_MAX
	};

	static std::vector<String> parameter_names() {
		return {
			"detection_range",
			"detection_angle",
			"repulsion",
			"clumping",
			"aligning",
			"damping",
			"scale",
			"anim_speed",
		};
	}

	enum Flags {
		FLAG_MAX
	};

	static std::vector<String> flag_names() {
		return {
		};
	}

private:

	// warning - beware of adding non-trivial types
	// to this structure as it is zeroed to initialize in set_amount()
	struct Particle {
		Transform2D transform;
		Color color;
		float custom[4];
		float rotation;
		Vector2 velocity;
		bool active;
		Color base_color;

	};

	RandomCustom random;

	float time;
	bool redraw;

	RID mesh;
	RID multimesh;

	PoolVector<Particle> particles;
	PoolVector<float> particle_data;

	Ref<Texture> texture;
	Ref<Texture> normalmap;

	////////

	float parameters[PARAM_MAX];
	float randomness[PARAM_MAX];

	Color color;

	bool flags[FLAG_MAX];

	void _update_internal();
	void _particles_process(float p_delta);
	void _update_particle_data_buffer();

	Mutex *update_mutex;

	void _update_render_thread();

	void _update_mesh_texture();

	void _set_redraw(bool p_redraw);

	void _texture_changed();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:

	void set_amount(int p_amount);

	int get_amount() const;

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

	void set_normalmap(const Ref<Texture> &p_normalmap);
	Ref<Texture> get_normalmap() const;

	///////////////////

	void set_param(Parameter p_param, float p_value);
	float get_param(Parameter p_param) const;

	void set_param_randomness(Parameter p_param, float p_value);
	float get_param_randomness(Parameter p_param) const;

	void set_color(const Color &p_color);
	Color get_color() const;

	void set_particle_flag(Flags p_flag, bool p_enable);
	bool get_particle_flag(Flags p_flag) const;

	virtual String get_configuration_warning() const;

	void restart();

	BoidParticles();
	~BoidParticles();
};

VARIANT_ENUM_CAST(BoidParticles::Parameter)
VARIANT_ENUM_CAST(BoidParticles::Flags)

#endif // BOID_PARTICLES_H
