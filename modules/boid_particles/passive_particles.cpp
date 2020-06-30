/*************************************************************************/
/*  passive_particles.cpp                                                */
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

#include "passive_particles.h"
#include "core/core_string_names.h"
#include "scene/2d/canvas_item.h"
#include "scene/2d/particles_2d.h"
#include "scene/resources/particles_material.h"
#include "servers/visual_server.h"

void PassiveParticles::set_emitting(bool p_emitting) {

	if (emitting == p_emitting)
		return;

	emitting = p_emitting;
	if (emitting)
		set_process_internal(true);
}

void PassiveParticles::set_amount(int p_amount) {

	ERR_FAIL_COND_MSG(p_amount < 1, "Amount of particles must be greater than 0.");

	particles.resize(p_amount);
	{
		PoolVector<Particle>::Write w = particles.write();

		// each particle must be set to false
		// zeroing the data also prevents uninitialized memory being sent to GPU
		zeromem(static_cast<void *>(&w[0]), p_amount * sizeof(Particle));
		// cast to prevent compiler warning .. note this relies on Particle not containing any complex types.
		// an alternative is to use some zero method per item but the generated code will be far less efficient.
	}

	particle_data.resize((8 + 4 + 1) * p_amount);
	VS::get_singleton()->multimesh_allocate(multimesh, p_amount, VS::MULTIMESH_TRANSFORM_2D, VS::MULTIMESH_COLOR_8BIT, VS::MULTIMESH_CUSTOM_DATA_FLOAT);

	particle_order.resize(p_amount);
}
void PassiveParticles::set_lifetime(float p_lifetime) {

	ERR_FAIL_COND_MSG(p_lifetime <= 0, "Particles lifetime must be greater than 0.");
	lifetime = p_lifetime;
}

void PassiveParticles::set_one_shot(bool p_one_shot) {

	one_shot = p_one_shot;
}

void PassiveParticles::set_pre_process_time(float p_time) {

	pre_process_time = p_time;
}
void PassiveParticles::set_explosiveness_ratio(float p_ratio) {

	explosiveness_ratio = p_ratio;
}
void PassiveParticles::set_randomness_ratio(float p_ratio) {

	randomness_ratio = p_ratio;
}
void PassiveParticles::set_lifetime_randomness(float p_random) {

	lifetime_randomness = p_random;
}
void PassiveParticles::set_use_local_coordinates(bool p_enable) {

	local_coords = p_enable;
	set_notify_transform(!p_enable);
}

void PassiveParticles::set_speed_scale(float p_scale) {

	speed_scale = p_scale;
}

bool PassiveParticles::is_emitting() const {

	return emitting;
}
int PassiveParticles::get_amount() const {

	return particles.size();
}
float PassiveParticles::get_lifetime() const {

	return lifetime;
}
bool PassiveParticles::get_one_shot() const {

	return one_shot;
}

float PassiveParticles::get_pre_process_time() const {

	return pre_process_time;
}
float PassiveParticles::get_explosiveness_ratio() const {

	return explosiveness_ratio;
}
float PassiveParticles::get_randomness_ratio() const {

	return randomness_ratio;
}
float PassiveParticles::get_lifetime_randomness() const {

	return lifetime_randomness;
}

bool PassiveParticles::get_use_local_coordinates() const {

	return local_coords;
}

float PassiveParticles::get_speed_scale() const {

	return speed_scale;
}

void PassiveParticles::set_draw_order(DrawOrder p_order) {

	draw_order = p_order;
}

PassiveParticles::DrawOrder PassiveParticles::get_draw_order() const {

	return draw_order;
}

void PassiveParticles::_update_mesh_texture() {

	Size2 tex_size;
	if (texture.is_valid()) {
		tex_size = texture->get_size();
	} else {
		tex_size = Size2(1, 1);
	}
	PoolVector<Vector2> vertices;
	vertices.push_back(-tex_size * 0.5);
	vertices.push_back(-tex_size * 0.5 + Vector2(tex_size.x, 0));
	vertices.push_back(-tex_size * 0.5 + Vector2(tex_size.x, tex_size.y));
	vertices.push_back(-tex_size * 0.5 + Vector2(0, tex_size.y));
	PoolVector<Vector2> uvs;
	AtlasTexture *atlas_texure = Object::cast_to<AtlasTexture>(*texture);
	if (atlas_texure && atlas_texure->get_atlas().is_valid()) {
		Rect2 region_rect = atlas_texure->get_region();
		Size2 atlas_size = atlas_texure->get_atlas()->get_size();
		uvs.push_back(Vector2(region_rect.position.x / atlas_size.x, region_rect.position.y / atlas_size.y));
		uvs.push_back(Vector2((region_rect.position.x + region_rect.size.x) / atlas_size.x, region_rect.position.y / atlas_size.y));
		uvs.push_back(Vector2((region_rect.position.x + region_rect.size.x) / atlas_size.x, (region_rect.position.y + region_rect.size.y) / atlas_size.y));
		uvs.push_back(Vector2(region_rect.position.x / atlas_size.x, (region_rect.position.y + region_rect.size.y) / atlas_size.y));
	} else {
		uvs.push_back(Vector2(0, 0));
		uvs.push_back(Vector2(1, 0));
		uvs.push_back(Vector2(1, 1));
		uvs.push_back(Vector2(0, 1));
	}
	PoolVector<Color> colors;
	colors.push_back(Color(1, 1, 1, 1));
	colors.push_back(Color(1, 1, 1, 1));
	colors.push_back(Color(1, 1, 1, 1));
	colors.push_back(Color(1, 1, 1, 1));
	PoolVector<int> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	Array arr;
	arr.resize(VS::ARRAY_MAX);
	arr[VS::ARRAY_VERTEX] = vertices;
	arr[VS::ARRAY_TEX_UV] = uvs;
	arr[VS::ARRAY_COLOR] = colors;
	arr[VS::ARRAY_INDEX] = indices;

	VS::get_singleton()->mesh_clear(mesh);
	VS::get_singleton()->mesh_add_surface_from_arrays(mesh, VS::PRIMITIVE_TRIANGLES, arr);
}

void PassiveParticles::set_texture(const Ref<Texture> &p_texture) {
	if (p_texture == texture)
		return;

	if (texture.is_valid())
		texture->disconnect(CoreStringNames::get_singleton()->changed, this, "_texture_changed");

	texture = p_texture;

	if (texture.is_valid())
		texture->connect(CoreStringNames::get_singleton()->changed, this, "_texture_changed");

	update();
	_update_mesh_texture();
}

void PassiveParticles::_texture_changed() {

	if (texture.is_valid()) {
		update();
		_update_mesh_texture();
	}
}

Ref<Texture> PassiveParticles::get_texture() const {

	return texture;
}

void PassiveParticles::set_normalmap(const Ref<Texture> &p_normalmap) {

	normalmap = p_normalmap;
	update();
}

Ref<Texture> PassiveParticles::get_normalmap() const {

	return normalmap;
}

void PassiveParticles::set_fixed_fps(int p_count) {
	fixed_fps = p_count;
}

int PassiveParticles::get_fixed_fps() const {
	return fixed_fps;
}

void PassiveParticles::set_fractional_delta(bool p_enable) {
	fractional_delta = p_enable;
}

bool PassiveParticles::get_fractional_delta() const {
	return fractional_delta;
}

String PassiveParticles::get_configuration_warning() const {

	String warnings;

	CanvasItemMaterial *mat = Object::cast_to<CanvasItemMaterial>(get_material().ptr());

	if (get_material().is_null() || (mat && !mat->get_particles_animation())) {
		if (get_param(PARAM_ANIM_SPEED) != 0.0 || get_param(PARAM_ANIM_OFFSET) != 0.0 ||
				get_param_curve(PARAM_ANIM_SPEED).is_valid() || get_param_curve(PARAM_ANIM_OFFSET).is_valid()) {
			if (warnings != String())
				warnings += "\n";
			warnings += "- " + TTR("PassiveParticles animation requires the usage of a CanvasItemMaterial with \"Particles Animation\" enabled.");
		}
	}

	return warnings;
}

void PassiveParticles::restart() {

	time = 0;
	inactive_time = 0;
	frame_remainder = 0;
	cycle = 0;
	emitting = false;

	{
		int pc = particles.size();
		PoolVector<Particle>::Write w = particles.write();

		for (int i = 0; i < pc; i++) {
			w[i].active = false;
		}
	}

	set_emitting(true);
}

void PassiveParticles::set_direction(Vector2 p_direction) {

	direction = p_direction;
}

Vector2 PassiveParticles::get_direction() const {

	return direction;
}

void PassiveParticles::set_spread(float p_spread) {

	spread = p_spread;
}

float PassiveParticles::get_spread() const {

	return spread;
}

void PassiveParticles::set_param(Parameter p_param, float p_value) {

	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	parameters[p_param] = p_value;
}
float PassiveParticles::get_param(Parameter p_param) const {

	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return parameters[p_param];
}

void PassiveParticles::set_param_randomness(Parameter p_param, float p_value) {

	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	randomness[p_param] = p_value;
}
float PassiveParticles::get_param_randomness(Parameter p_param) const {

	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return randomness[p_param];
}

static void _adjust_curve_range(const Ref<Curve> &p_curve, float p_min, float p_max) {

	Ref<Curve> curve = p_curve;
	if (!curve.is_valid())
		return;

	curve->ensure_default_setup(p_min, p_max);
}

void PassiveParticles::set_param_curve(Parameter p_param, const Ref<Curve> &p_curve) {

	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	curve_parameters[p_param] = p_curve;

	switch (p_param) {
		case PARAM_INITIAL_LINEAR_VELOCITY: {
			//do none for this one
		} break;
		case PARAM_ANGULAR_VELOCITY: {
			_adjust_curve_range(p_curve, -360, 360);
		} break;
		case PARAM_ORBIT_VELOCITY: {
			_adjust_curve_range(p_curve, -500, 500);
		} break;
		case PARAM_LINEAR_ACCEL: {
			_adjust_curve_range(p_curve, -200, 200);
		} break;
		case PARAM_RADIAL_ACCEL: {
			_adjust_curve_range(p_curve, -200, 200);
		} break;
		case PARAM_TANGENTIAL_ACCEL: {
			_adjust_curve_range(p_curve, -200, 200);
		} break;
		case PARAM_DAMPING: {
			_adjust_curve_range(p_curve, 0, 100);
		} break;
		case PARAM_ANGLE: {
			_adjust_curve_range(p_curve, -360, 360);
		} break;
		case PARAM_SCALE: {

		} break;
		case PARAM_HUE_VARIATION: {
			_adjust_curve_range(p_curve, -1, 1);
		} break;
		case PARAM_ANIM_SPEED: {
			_adjust_curve_range(p_curve, 0, 200);
		} break;
		case PARAM_ANIM_OFFSET: {
		} break;
		default: {
		}
	}
}
Ref<Curve> PassiveParticles::get_param_curve(Parameter p_param) const {

	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, Ref<Curve>());

	return curve_parameters[p_param];
}

void PassiveParticles::set_color(const Color &p_color) {

	color = p_color;
}

Color PassiveParticles::get_color() const {

	return color;
}

void PassiveParticles::set_color_ramp(const Ref<Gradient> &p_ramp) {

	color_ramp = p_ramp;
}

Ref<Gradient> PassiveParticles::get_color_ramp() const {

	return color_ramp;
}

void PassiveParticles::set_particle_flag(Flags p_flag, bool p_enable) {
	ERR_FAIL_INDEX(p_flag, FLAG_MAX);
	flags[p_flag] = p_enable;
}

bool PassiveParticles::get_particle_flag(Flags p_flag) const {
	ERR_FAIL_INDEX_V(p_flag, FLAG_MAX, false);
	return flags[p_flag];
}

void PassiveParticles::set_emission_shape(EmissionShape p_shape) {
	ERR_FAIL_INDEX(p_shape, EMISSION_SHAPE_MAX);
	emission_shape = p_shape;
	_change_notify();
}

void PassiveParticles::set_emission_sphere_radius(float p_radius) {

	emission_sphere_radius = p_radius;
}

void PassiveParticles::set_emission_rect_extents(Vector2 p_extents) {

	emission_rect_extents = p_extents;
}

void PassiveParticles::set_emission_points(const PoolVector<Vector2> &p_points) {

	emission_points = p_points;
}

void PassiveParticles::set_emission_normals(const PoolVector<Vector2> &p_normals) {

	emission_normals = p_normals;
}

void PassiveParticles::set_emission_colors(const PoolVector<Color> &p_colors) {

	emission_colors = p_colors;
}

float PassiveParticles::get_emission_sphere_radius() const {

	return emission_sphere_radius;
}
Vector2 PassiveParticles::get_emission_rect_extents() const {

	return emission_rect_extents;
}
PoolVector<Vector2> PassiveParticles::get_emission_points() const {

	return emission_points;
}
PoolVector<Vector2> PassiveParticles::get_emission_normals() const {

	return emission_normals;
}

PoolVector<Color> PassiveParticles::get_emission_colors() const {

	return emission_colors;
}

PassiveParticles::EmissionShape PassiveParticles::get_emission_shape() const {
	return emission_shape;
}
void PassiveParticles::set_gravity(const Vector2 &p_gravity) {

	gravity = p_gravity;
}

Vector2 PassiveParticles::get_gravity() const {

	return gravity;
}

void PassiveParticles::_validate_property(PropertyInfo &property) const {

	if (property.name == "color" && color_ramp.is_valid()) {
		property.usage = 0;
	}

	if (property.name == "emission_sphere_radius" && emission_shape != EMISSION_SHAPE_SPHERE) {
		property.usage = 0;
	}

	if (property.name == "emission_rect_extents" && emission_shape != EMISSION_SHAPE_RECTANGLE) {
		property.usage = 0;
	}

	if ((property.name == "emission_point_texture" || property.name == "emission_color_texture") && (emission_shape < EMISSION_SHAPE_POINTS)) {
		property.usage = 0;
	}

	if (property.name == "emission_normals" && emission_shape != EMISSION_SHAPE_DIRECTED_POINTS) {
		property.usage = 0;
	}

	if (property.name == "emission_points" && emission_shape != EMISSION_SHAPE_POINTS && emission_shape != EMISSION_SHAPE_DIRECTED_POINTS) {
		property.usage = 0;
	}

	if (property.name == "emission_colors" && emission_shape != EMISSION_SHAPE_POINTS && emission_shape != EMISSION_SHAPE_DIRECTED_POINTS) {
		property.usage = 0;
	}
}

static uint32_t idhash(uint32_t x) {

	x = ((x >> uint32_t(16)) ^ x) * uint32_t(0x45d9f3b);
	x = ((x >> uint32_t(16)) ^ x) * uint32_t(0x45d9f3b);
	x = (x >> uint32_t(16)) ^ x;
	return x;
}

static float rand_from_seed(uint32_t &seed) {
	int k;
	int s = int(seed);
	if (s == 0)
		s = 305420679;
	k = s / 127773;
	s = 16807 * (s - k * 127773) - 2836 * k;
	if (s < 0)
		s += 2147483647;
	seed = uint32_t(s);
	return float(seed % uint32_t(65536)) / 65535.0;
}

void PassiveParticles::_update_internal() {

	if (particles.size() == 0 || !is_visible_in_tree()) {
		_set_redraw(false);
		return;
	}

	float delta = get_process_delta_time();
	if (emitting) {
		inactive_time = 0;
	} else {
		inactive_time += delta;
		if (inactive_time > lifetime * 1.2) {
			set_process_internal(false);
			_set_redraw(false);

			//reset variables
			time = 0;
			inactive_time = 0;
			frame_remainder = 0;
			cycle = 0;
			return;
		}
	}
	_set_redraw(true);

	if (time == 0 && pre_process_time > 0.0) {

		float frame_time;
		if (fixed_fps > 0)
			frame_time = 1.0 / fixed_fps;
		else
			frame_time = 1.0 / 30.0;

		float todo = pre_process_time;

		while (todo >= 0) {
			_particles_process(frame_time);
			todo -= frame_time;
		}
	}

	if (fixed_fps > 0) {
		float frame_time = 1.0 / fixed_fps;
		float decr = frame_time;

		float ldelta = delta;
		if (ldelta > 0.1) { //avoid recursive stalls if fps goes below 10
			ldelta = 0.1;
		} else if (ldelta <= 0.0) { //unlikely but..
			ldelta = 0.001;
		}
		float todo = frame_remainder + ldelta;

		while (todo >= frame_time) {
			_particles_process(frame_time);
			todo -= decr;
		}

		frame_remainder = todo;

	} else {
		_particles_process(delta);
	}

	_update_particle_data_buffer();
}

void PassiveParticles::_particles_process(float p_delta) {

}

void PassiveParticles::_update_particle_data_buffer() {
#ifndef NO_THREADS
	update_mutex->lock();
#endif

	{

		int pc = particles.size();

		PoolVector<int>::Write ow;
		int *order = NULL;

		PoolVector<float>::Write w = particle_data.write();
		PoolVector<Particle>::Read r = particles.read();
		float *ptr = w.ptr();

		if (draw_order != DRAW_ORDER_INDEX) {
			ow = particle_order.write();
			order = ow.ptr();

			for (int i = 0; i < pc; i++) {
				order[i] = i;
			}
			if (draw_order == DRAW_ORDER_LIFETIME) {
				SortArray<int, SortLifetime> sorter;
				sorter.compare.particles = r.ptr();
				sorter.sort(order, pc);
			}
		}

		for (int i = 0; i < pc; i++) {

			int idx = order ? order[i] : i;

			Transform2D t = r[idx].transform;

			if (!local_coords) {
				t = inv_emission_transform * t;
			}

			if (r[idx].active) {

				ptr[0] = t.elements[0][0];
				ptr[1] = t.elements[1][0];
				ptr[2] = 0;
				ptr[3] = t.elements[2][0];
				ptr[4] = t.elements[0][1];
				ptr[5] = t.elements[1][1];
				ptr[6] = 0;
				ptr[7] = t.elements[2][1];

				Color c = r[idx].color;
				uint8_t *data8 = (uint8_t *)&ptr[8];
				data8[0] = CLAMP(c.r * 255.0, 0, 255);
				data8[1] = CLAMP(c.g * 255.0, 0, 255);
				data8[2] = CLAMP(c.b * 255.0, 0, 255);
				data8[3] = CLAMP(c.a * 255.0, 0, 255);

				ptr[9] = r[idx].custom[0];
				ptr[10] = r[idx].custom[1];
				ptr[11] = r[idx].custom[2];
				ptr[12] = r[idx].custom[3];

			} else {
				zeromem(ptr, sizeof(float) * 13);
			}

			ptr += 13;
		}
	}

#ifndef NO_THREADS
	update_mutex->unlock();
#endif
}

void PassiveParticles::_set_redraw(bool p_redraw) {
	if (redraw == p_redraw)
		return;
	redraw = p_redraw;
#ifndef NO_THREADS
	update_mutex->lock();
#endif
	if (redraw) {
		VS::get_singleton()->connect("frame_pre_draw", this, "_update_render_thread");
		VS::get_singleton()->canvas_item_set_update_when_visible(get_canvas_item(), true);

		VS::get_singleton()->multimesh_set_visible_instances(multimesh, -1);
	} else {
		if (VS::get_singleton()->is_connected("frame_pre_draw", this, "_update_render_thread")) {
			VS::get_singleton()->disconnect("frame_pre_draw", this, "_update_render_thread");
		}
		VS::get_singleton()->canvas_item_set_update_when_visible(get_canvas_item(), false);

		VS::get_singleton()->multimesh_set_visible_instances(multimesh, 0);
	}
#ifndef NO_THREADS
	update_mutex->unlock();
#endif
	update(); // redraw to update render list
}

void PassiveParticles::_update_render_thread() {

#ifndef NO_THREADS
	update_mutex->lock();
#endif

	VS::get_singleton()->multimesh_set_as_bulk_array(multimesh, particle_data);

#ifndef NO_THREADS
	update_mutex->unlock();
#endif
}

void PassiveParticles::_notification(int p_what) {

	if (p_what == NOTIFICATION_ENTER_TREE) {
		set_process_internal(emitting);
	}

	if (p_what == NOTIFICATION_EXIT_TREE) {
		_set_redraw(false);
	}

	if (p_what == NOTIFICATION_DRAW) {
		// first update before rendering to avoid one frame delay after emitting starts
		if (emitting && (time == 0))
			_update_internal();

		if (!redraw)
			return; // don't add to render list

		RID texrid;
		if (texture.is_valid()) {
			texrid = texture->get_rid();
		}

		RID normrid;
		if (normalmap.is_valid()) {
			normrid = normalmap->get_rid();
		}

		VS::get_singleton()->canvas_item_add_multimesh(get_canvas_item(), multimesh, texrid, normrid);
	}

	if (p_what == NOTIFICATION_INTERNAL_PROCESS) {
		_update_internal();
	}

	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {

		inv_emission_transform = get_global_transform().affine_inverse();

		if (!local_coords) {

			int pc = particles.size();

			PoolVector<float>::Write w = particle_data.write();
			PoolVector<Particle>::Read r = particles.read();
			float *ptr = w.ptr();

			for (int i = 0; i < pc; i++) {

				Transform2D t = inv_emission_transform * r[i].transform;

				if (r[i].active) {

					ptr[0] = t.elements[0][0];
					ptr[1] = t.elements[1][0];
					ptr[2] = 0;
					ptr[3] = t.elements[2][0];
					ptr[4] = t.elements[0][1];
					ptr[5] = t.elements[1][1];
					ptr[6] = 0;
					ptr[7] = t.elements[2][1];

				} else {
					zeromem(ptr, sizeof(float) * 8);
				}

				ptr += 13;
			}
		}
	}
}

void PassiveParticles::convert_from_particles(Node *p_particles) {

	Particles2D *particles = Object::cast_to<Particles2D>(p_particles);
	ERR_FAIL_COND_MSG(!particles, "Only Particles2D nodes can be converted to PassiveParticles.");

	set_emitting(particles->is_emitting());
	set_amount(particles->get_amount());
	set_lifetime(particles->get_lifetime());
	set_one_shot(particles->get_one_shot());
	set_pre_process_time(particles->get_pre_process_time());
	set_explosiveness_ratio(particles->get_explosiveness_ratio());
	set_randomness_ratio(particles->get_randomness_ratio());
	set_use_local_coordinates(particles->get_use_local_coordinates());
	set_fixed_fps(particles->get_fixed_fps());
	set_fractional_delta(particles->get_fractional_delta());
	set_speed_scale(particles->get_speed_scale());
	set_draw_order(DrawOrder(particles->get_draw_order()));
	set_texture(particles->get_texture());

	Ref<Material> mat = particles->get_material();
	if (mat.is_valid()) {
		set_material(mat);
	}

	Ref<ParticlesMaterial> material = particles->get_process_material();
	if (material.is_null())
		return;

	Vector3 dir = material->get_direction();
	set_direction(Vector2(dir.x, dir.y));
	set_spread(material->get_spread());

	set_color(material->get_color());

	Ref<GradientTexture> gt = material->get_color_ramp();
	if (gt.is_valid()) {
		set_color_ramp(gt->get_gradient());
	}

	set_particle_flag(FLAG_ALIGN_Y_TO_VELOCITY, material->get_flag(ParticlesMaterial::FLAG_ALIGN_Y_TO_VELOCITY));

	set_emission_shape(EmissionShape(material->get_emission_shape()));
	set_emission_sphere_radius(material->get_emission_sphere_radius());
	Vector2 rect_extents = Vector2(material->get_emission_box_extents().x, material->get_emission_box_extents().y);
	set_emission_rect_extents(rect_extents);

	Vector2 gravity = Vector2(material->get_gravity().x, material->get_gravity().y);
	set_gravity(gravity);
	set_lifetime_randomness(material->get_lifetime_randomness());

#define CONVERT_PARAM(m_param)                                                            \
	set_param(m_param, material->get_param(ParticlesMaterial::m_param));                  \
	{                                                                                     \
		Ref<CurveTexture> ctex = material->get_param_texture(ParticlesMaterial::m_param); \
		if (ctex.is_valid()) set_param_curve(m_param, ctex->get_curve());                 \
	}                                                                                     \
	set_param_randomness(m_param, material->get_param_randomness(ParticlesMaterial::m_param));

	CONVERT_PARAM(PARAM_INITIAL_LINEAR_VELOCITY);
	CONVERT_PARAM(PARAM_ANGULAR_VELOCITY);
	CONVERT_PARAM(PARAM_ORBIT_VELOCITY);
	CONVERT_PARAM(PARAM_LINEAR_ACCEL);
	CONVERT_PARAM(PARAM_RADIAL_ACCEL);
	CONVERT_PARAM(PARAM_TANGENTIAL_ACCEL);
	CONVERT_PARAM(PARAM_DAMPING);
	CONVERT_PARAM(PARAM_ANGLE);
	CONVERT_PARAM(PARAM_SCALE);
	CONVERT_PARAM(PARAM_HUE_VARIATION);
	CONVERT_PARAM(PARAM_ANIM_SPEED);
	CONVERT_PARAM(PARAM_ANIM_OFFSET);

#undef CONVERT_PARAM
}

void PassiveParticles::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_emitting", "emitting"), &PassiveParticles::set_emitting);
	ClassDB::bind_method(D_METHOD("set_amount", "amount"), &PassiveParticles::set_amount);
	ClassDB::bind_method(D_METHOD("set_lifetime", "secs"), &PassiveParticles::set_lifetime);
	ClassDB::bind_method(D_METHOD("set_one_shot", "enable"), &PassiveParticles::set_one_shot);
	ClassDB::bind_method(D_METHOD("set_pre_process_time", "secs"), &PassiveParticles::set_pre_process_time);
	ClassDB::bind_method(D_METHOD("set_explosiveness_ratio", "ratio"), &PassiveParticles::set_explosiveness_ratio);
	ClassDB::bind_method(D_METHOD("set_randomness_ratio", "ratio"), &PassiveParticles::set_randomness_ratio);
	ClassDB::bind_method(D_METHOD("set_lifetime_randomness", "random"), &PassiveParticles::set_lifetime_randomness);
	ClassDB::bind_method(D_METHOD("set_use_local_coordinates", "enable"), &PassiveParticles::set_use_local_coordinates);
	ClassDB::bind_method(D_METHOD("set_fixed_fps", "fps"), &PassiveParticles::set_fixed_fps);
	ClassDB::bind_method(D_METHOD("set_fractional_delta", "enable"), &PassiveParticles::set_fractional_delta);
	ClassDB::bind_method(D_METHOD("set_speed_scale", "scale"), &PassiveParticles::set_speed_scale);

	ClassDB::bind_method(D_METHOD("is_emitting"), &PassiveParticles::is_emitting);
	ClassDB::bind_method(D_METHOD("get_amount"), &PassiveParticles::get_amount);
	ClassDB::bind_method(D_METHOD("get_lifetime"), &PassiveParticles::get_lifetime);
	ClassDB::bind_method(D_METHOD("get_one_shot"), &PassiveParticles::get_one_shot);
	ClassDB::bind_method(D_METHOD("get_pre_process_time"), &PassiveParticles::get_pre_process_time);
	ClassDB::bind_method(D_METHOD("get_explosiveness_ratio"), &PassiveParticles::get_explosiveness_ratio);
	ClassDB::bind_method(D_METHOD("get_randomness_ratio"), &PassiveParticles::get_randomness_ratio);
	ClassDB::bind_method(D_METHOD("get_lifetime_randomness"), &PassiveParticles::get_lifetime_randomness);
	ClassDB::bind_method(D_METHOD("get_use_local_coordinates"), &PassiveParticles::get_use_local_coordinates);
	ClassDB::bind_method(D_METHOD("get_fixed_fps"), &PassiveParticles::get_fixed_fps);
	ClassDB::bind_method(D_METHOD("get_fractional_delta"), &PassiveParticles::get_fractional_delta);
	ClassDB::bind_method(D_METHOD("get_speed_scale"), &PassiveParticles::get_speed_scale);

	ClassDB::bind_method(D_METHOD("set_draw_order", "order"), &PassiveParticles::set_draw_order);

	ClassDB::bind_method(D_METHOD("get_draw_order"), &PassiveParticles::get_draw_order);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &PassiveParticles::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &PassiveParticles::get_texture);

	ClassDB::bind_method(D_METHOD("set_normalmap", "normalmap"), &PassiveParticles::set_normalmap);
	ClassDB::bind_method(D_METHOD("get_normalmap"), &PassiveParticles::get_normalmap);

	ClassDB::bind_method(D_METHOD("restart"), &PassiveParticles::restart);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "emitting"), "set_emitting", "is_emitting");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "amount", PROPERTY_HINT_EXP_RANGE, "1,1000000,1"), "set_amount", "get_amount");
	ADD_GROUP("Time", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime", PROPERTY_HINT_RANGE, "0.01,600.0,0.01,or_greater"), "set_lifetime", "get_lifetime");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "one_shot"), "set_one_shot", "get_one_shot");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "preprocess", PROPERTY_HINT_RANGE, "0.00,600.0,0.01"), "set_pre_process_time", "get_pre_process_time");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed_scale", PROPERTY_HINT_RANGE, "0,64,0.01"), "set_speed_scale", "get_speed_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "explosiveness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_explosiveness_ratio", "get_explosiveness_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "randomness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_randomness_ratio", "get_randomness_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime_randomness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_lifetime_randomness", "get_lifetime_randomness");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fixed_fps", PROPERTY_HINT_RANGE, "0,1000,1"), "set_fixed_fps", "get_fixed_fps");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fract_delta"), "set_fractional_delta", "get_fractional_delta");
	ADD_GROUP("Drawing", "");
	// No visibility_rect property contrarily to Particles2D, it's updated automatically.
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "local_coords"), "set_use_local_coordinates", "get_use_local_coordinates");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "draw_order", PROPERTY_HINT_ENUM, "Index,Lifetime"), "set_draw_order", "get_draw_order");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normalmap", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_normalmap", "get_normalmap");

	BIND_ENUM_CONSTANT(DRAW_ORDER_INDEX);
	BIND_ENUM_CONSTANT(DRAW_ORDER_LIFETIME);

	////////////////////////////////

	ClassDB::bind_method(D_METHOD("set_direction", "direction"), &PassiveParticles::set_direction);
	ClassDB::bind_method(D_METHOD("get_direction"), &PassiveParticles::get_direction);

	ClassDB::bind_method(D_METHOD("set_spread", "degrees"), &PassiveParticles::set_spread);
	ClassDB::bind_method(D_METHOD("get_spread"), &PassiveParticles::get_spread);

	ClassDB::bind_method(D_METHOD("set_param", "param", "value"), &PassiveParticles::set_param);
	ClassDB::bind_method(D_METHOD("get_param", "param"), &PassiveParticles::get_param);

	ClassDB::bind_method(D_METHOD("set_param_randomness", "param", "randomness"), &PassiveParticles::set_param_randomness);
	ClassDB::bind_method(D_METHOD("get_param_randomness", "param"), &PassiveParticles::get_param_randomness);

	ClassDB::bind_method(D_METHOD("set_param_curve", "param", "curve"), &PassiveParticles::set_param_curve);
	ClassDB::bind_method(D_METHOD("get_param_curve", "param"), &PassiveParticles::get_param_curve);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &PassiveParticles::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &PassiveParticles::get_color);

	ClassDB::bind_method(D_METHOD("set_color_ramp", "ramp"), &PassiveParticles::set_color_ramp);
	ClassDB::bind_method(D_METHOD("get_color_ramp"), &PassiveParticles::get_color_ramp);

	ClassDB::bind_method(D_METHOD("set_particle_flag", "flag", "enable"), &PassiveParticles::set_particle_flag);
	ClassDB::bind_method(D_METHOD("get_particle_flag", "flag"), &PassiveParticles::get_particle_flag);

	ClassDB::bind_method(D_METHOD("set_emission_shape", "shape"), &PassiveParticles::set_emission_shape);
	ClassDB::bind_method(D_METHOD("get_emission_shape"), &PassiveParticles::get_emission_shape);

	ClassDB::bind_method(D_METHOD("set_emission_sphere_radius", "radius"), &PassiveParticles::set_emission_sphere_radius);
	ClassDB::bind_method(D_METHOD("get_emission_sphere_radius"), &PassiveParticles::get_emission_sphere_radius);

	ClassDB::bind_method(D_METHOD("set_emission_rect_extents", "extents"), &PassiveParticles::set_emission_rect_extents);
	ClassDB::bind_method(D_METHOD("get_emission_rect_extents"), &PassiveParticles::get_emission_rect_extents);

	ClassDB::bind_method(D_METHOD("set_emission_points", "array"), &PassiveParticles::set_emission_points);
	ClassDB::bind_method(D_METHOD("get_emission_points"), &PassiveParticles::get_emission_points);

	ClassDB::bind_method(D_METHOD("set_emission_normals", "array"), &PassiveParticles::set_emission_normals);
	ClassDB::bind_method(D_METHOD("get_emission_normals"), &PassiveParticles::get_emission_normals);

	ClassDB::bind_method(D_METHOD("set_emission_colors", "array"), &PassiveParticles::set_emission_colors);
	ClassDB::bind_method(D_METHOD("get_emission_colors"), &PassiveParticles::get_emission_colors);

	ClassDB::bind_method(D_METHOD("get_gravity"), &PassiveParticles::get_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity", "accel_vec"), &PassiveParticles::set_gravity);

	ClassDB::bind_method(D_METHOD("convert_from_particles", "particles"), &PassiveParticles::convert_from_particles);

	ClassDB::bind_method(D_METHOD("_update_render_thread"), &PassiveParticles::_update_render_thread);
	ClassDB::bind_method(D_METHOD("_texture_changed"), &PassiveParticles::_texture_changed);

	ADD_GROUP("Emission Shape", "emission_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "emission_shape", PROPERTY_HINT_ENUM, "Point,Sphere,Box,Points,Directed Points"), "set_emission_shape", "get_emission_shape");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "emission_sphere_radius", PROPERTY_HINT_RANGE, "0.01,128,0.01"), "set_emission_sphere_radius", "get_emission_sphere_radius");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "emission_rect_extents"), "set_emission_rect_extents", "get_emission_rect_extents");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "emission_points"), "set_emission_points", "get_emission_points");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "emission_normals"), "set_emission_normals", "get_emission_normals");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_COLOR_ARRAY, "emission_colors"), "set_emission_colors", "get_emission_colors");
	ADD_GROUP("Flags", "flag_");
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "flag_align_y"), "set_particle_flag", "get_particle_flag", FLAG_ALIGN_Y_TO_VELOCITY);
	ADD_GROUP("Direction", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "direction"), "set_direction", "get_direction");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "spread", PROPERTY_HINT_RANGE, "0,180,0.01"), "set_spread", "get_spread");
	ADD_GROUP("Gravity", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "gravity"), "set_gravity", "get_gravity");
	ADD_GROUP("Initial Velocity", "initial_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "initial_velocity", PROPERTY_HINT_RANGE, "0,1000,0.01,or_greater"), "set_param", "get_param", PARAM_INITIAL_LINEAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "initial_velocity_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_INITIAL_LINEAR_VELOCITY);
	ADD_GROUP("Angular Velocity", "angular_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angular_velocity", PROPERTY_HINT_RANGE, "-720,720,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_ANGULAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angular_velocity_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANGULAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "angular_velocity_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANGULAR_VELOCITY);
	ADD_GROUP("Orbit Velocity", "orbit_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "orbit_velocity", PROPERTY_HINT_RANGE, "-1000,1000,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_ORBIT_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "orbit_velocity_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ORBIT_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "orbit_velocity_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ORBIT_VELOCITY);
	ADD_GROUP("Linear Accel", "linear_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "linear_accel", PROPERTY_HINT_RANGE, "-100,100,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_LINEAR_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "linear_accel_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_LINEAR_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "linear_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_LINEAR_ACCEL);
	ADD_GROUP("Radial Accel", "radial_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "radial_accel", PROPERTY_HINT_RANGE, "-100,100,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_RADIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "radial_accel_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_RADIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "radial_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_RADIAL_ACCEL);
	ADD_GROUP("Tangential Accel", "tangential_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "tangential_accel", PROPERTY_HINT_RANGE, "-100,100,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_TANGENTIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "tangential_accel_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_TANGENTIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "tangential_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_TANGENTIAL_ACCEL);
	ADD_GROUP("Damping", "");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "damping", PROPERTY_HINT_RANGE, "0,100,0.01"), "set_param", "get_param", PARAM_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "damping_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "damping_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_DAMPING);
	ADD_GROUP("Angle", "");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angle", PROPERTY_HINT_RANGE, "-720,720,0.1,or_lesser,or_greater"), "set_param", "get_param", PARAM_ANGLE);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angle_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANGLE);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "angle_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANGLE);
	ADD_GROUP("Scale", "");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "scale_amount", PROPERTY_HINT_RANGE, "0,1000,0.01,or_greater"), "set_param", "get_param", PARAM_SCALE);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "scale_amount_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_SCALE);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "scale_amount_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_SCALE);
	ADD_GROUP("Color", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "color_ramp", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_color_ramp", "get_color_ramp");

	ADD_GROUP("Hue Variation", "hue_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "hue_variation", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_param", "get_param", PARAM_HUE_VARIATION);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "hue_variation_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_HUE_VARIATION);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "hue_variation_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_HUE_VARIATION);
	ADD_GROUP("Animation", "anim_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_speed", PROPERTY_HINT_RANGE, "0,128,0.01,or_greater"), "set_param", "get_param", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_speed_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "anim_speed_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_offset", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param", "get_param", PARAM_ANIM_OFFSET);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_offset_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANIM_OFFSET);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "anim_offset_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANIM_OFFSET);

	BIND_ENUM_CONSTANT(PARAM_INITIAL_LINEAR_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_ANGULAR_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_ORBIT_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_LINEAR_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_RADIAL_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_TANGENTIAL_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_DAMPING);
	BIND_ENUM_CONSTANT(PARAM_ANGLE);
	BIND_ENUM_CONSTANT(PARAM_SCALE);
	BIND_ENUM_CONSTANT(PARAM_HUE_VARIATION);
	BIND_ENUM_CONSTANT(PARAM_ANIM_SPEED);
	BIND_ENUM_CONSTANT(PARAM_ANIM_OFFSET);
	BIND_ENUM_CONSTANT(PARAM_MAX);

	BIND_ENUM_CONSTANT(FLAG_ALIGN_Y_TO_VELOCITY);
	BIND_ENUM_CONSTANT(FLAG_ROTATE_Y); // Unused, but exposed for consistency with 3D.
	BIND_ENUM_CONSTANT(FLAG_DISABLE_Z); // Unused, but exposed for consistency with 3D.
	BIND_ENUM_CONSTANT(FLAG_MAX);

	BIND_ENUM_CONSTANT(EMISSION_SHAPE_POINT);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_SPHERE);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_RECTANGLE);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_POINTS);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_DIRECTED_POINTS);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_MAX);
}

PassiveParticles::PassiveParticles() {

	time = 0;
	inactive_time = 0;
	frame_remainder = 0;
	cycle = 0;
	redraw = false;
	emitting = false;

	mesh = VisualServer::get_singleton()->mesh_create();
	multimesh = VisualServer::get_singleton()->multimesh_create();
	VisualServer::get_singleton()->multimesh_set_mesh(multimesh, mesh);

	set_emitting(true);
	set_one_shot(false);
	set_amount(8);
	set_lifetime(1);
	set_fixed_fps(0);
	set_fractional_delta(true);
	set_pre_process_time(0);
	set_explosiveness_ratio(0);
	set_randomness_ratio(0);
	set_lifetime_randomness(0);
	set_use_local_coordinates(true);

	set_draw_order(DRAW_ORDER_INDEX);
	set_speed_scale(1);

	set_direction(Vector2(1, 0));
	set_spread(45);
	set_param(PARAM_INITIAL_LINEAR_VELOCITY, 0);
	set_param(PARAM_ANGULAR_VELOCITY, 0);
	set_param(PARAM_ORBIT_VELOCITY, 0);
	set_param(PARAM_LINEAR_ACCEL, 0);
	set_param(PARAM_RADIAL_ACCEL, 0);
	set_param(PARAM_TANGENTIAL_ACCEL, 0);
	set_param(PARAM_DAMPING, 0);
	set_param(PARAM_ANGLE, 0);
	set_param(PARAM_SCALE, 1);
	set_param(PARAM_HUE_VARIATION, 0);
	set_param(PARAM_ANIM_SPEED, 0);
	set_param(PARAM_ANIM_OFFSET, 0);
	set_emission_shape(EMISSION_SHAPE_POINT);
	set_emission_sphere_radius(1);
	set_emission_rect_extents(Vector2(1, 1));

	set_gravity(Vector2(0, 98));

	for (int i = 0; i < PARAM_MAX; i++) {
		set_param_randomness(Parameter(i), 0);
	}

	for (int i = 0; i < FLAG_MAX; i++) {
		flags[i] = false;
	}

	set_color(Color(1, 1, 1, 1));

#ifndef NO_THREADS
	update_mutex = Mutex::create();
#endif

	_update_mesh_texture();
}

PassiveParticles::~PassiveParticles() {
	VS::get_singleton()->free(multimesh);
	VS::get_singleton()->free(mesh);

#ifndef NO_THREADS
	memdelete(update_mutex);
#endif
}
