/*************************************************************************/
/*  cpu_particles_2d.cpp                                                 */
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

#include <iostream>

#include "boid_particles.h"
#include "core/core_string_names.h"
#include "scene/2d/canvas_item.h"
#include "scene/2d/particles_2d.h"
#include "scene/resources/particles_material.h"
#include "servers/visual_server.h"

void BoidParticles::set_amount(int p_amount) {

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

}

int BoidParticles::get_amount() const {

	return particles.size();
}

void BoidParticles::_update_mesh_texture() {

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

void BoidParticles::set_texture(const Ref<Texture> &p_texture) {
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

void BoidParticles::_texture_changed() {

	if (texture.is_valid()) {
		update();
		_update_mesh_texture();
	}
}

Ref<Texture> BoidParticles::get_texture() const {

	return texture;
}

void BoidParticles::set_normalmap(const Ref<Texture> &p_normalmap) {

	normalmap = p_normalmap;
	update();
}

Ref<Texture> BoidParticles::get_normalmap() const {

	return normalmap;
}

String BoidParticles::get_configuration_warning() const {

	String warnings;

	CanvasItemMaterial *mat = Object::cast_to<CanvasItemMaterial>(get_material().ptr());

	if (get_material().is_null() || (mat && !mat->get_particles_animation())) {
		if (get_param(PARAM_ANIM_SPEED) != 0.0) {
			if (warnings != String())
				warnings += "\n";
			warnings += "- " + TTR("BoidParticles animation requires the usage of a CanvasItemMaterial with \"Particles Animation\" enabled.");
		}
	}

	return warnings;
}

void BoidParticles::restart() {
	time = 0;
	auto rect = get_viewport_rect();

	int pcount = particles.size();
	PoolVector<Particle>::Write w = particles.write();
	Particle *parray = w.ptr();
	for (int i = 0; i < pcount; i++) {
		Particle &p = parray[i];
		p.transform.elements[0] = Vector2(1.0, 0.0);
		p.transform.elements[1] = Vector2(0.0, 1.0);
		p.transform[2] = random.rand_within_rect(rect);
		p.custom[0] = 0.0;
		p.custom[1] = 0.0;
		p.custom[2] = 0.0;
		p.custom[3] = 0.0;
		p.rotation = 0.0;
		p.velocity = Vector2(0.0, 0.0);
		p.active = true;
		p.base_color = Color(1, 1, 1, 1);
	}

}

void BoidParticles::set_param(Parameter p_param, float p_value) {

	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	parameters[p_param] = p_value;
}
float BoidParticles::get_param(Parameter p_param) const {

	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return parameters[p_param];
}

void BoidParticles::set_param_randomness(Parameter p_param, float p_value) {

	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	randomness[p_param] = p_value;
}
float BoidParticles::get_param_randomness(Parameter p_param) const {

	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return randomness[p_param];
}

void BoidParticles::set_color(const Color &p_color) {

	color = p_color;
}

Color BoidParticles::get_color() const {

	return color;
}

void BoidParticles::set_particle_flag(Flags p_flag, bool p_enable) {
	ERR_FAIL_INDEX(p_flag, FLAG_MAX);
	flags[p_flag] = p_enable;
}

bool BoidParticles::get_particle_flag(Flags p_flag) const {
	ERR_FAIL_INDEX_V(p_flag, FLAG_MAX, false);
	return flags[p_flag];
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

void BoidParticles::_physics_process(float delta) {
	_particles_process(delta);
	_update_particle_data_buffer();
}

void BoidParticles::_particles_process(float p_delta) {

	int pcount = particles.size();
	PoolVector<Particle>::Write w = particles.write();

	Particle *parray = w.ptr();

	time += p_delta;

	for (int i = 0; i < pcount; i++) {

		Particle &p = parray[i];

		if (!p.active)
			continue;

		Vector2 pos = p.transform[2];

		auto average_position =	Vector2();
		auto average_velocity = Vector2();
		auto norm_factor = 0.0;

		auto force = Vector2();

		for (int j = 0; j < pcount; j++) {

			Particle &other_p = parray[j];

			if (!other_p.active)
				continue;

			auto other_pos = other_p.transform[2];

			auto dir = pos - other_pos;
			auto diff = dir.length();

			if (diff < parameters[PARAM_DETECTION_RANGE]) {

				auto inv_diff = 1.0 - diff / parameters[PARAM_DETECTION_RANGE];
				auto repulsion_factor = inv_diff * inv_diff * parameters[PARAM_REPULSION];

				average_position += other_pos;
				average_velocity += other_p.velocity;
				norm_factor += 1.0;

				force += p_delta * repulsion_factor * dir;

			}
		}

		average_position /= norm_factor;
		average_velocity /= norm_factor;
		force += p_delta * parameters[PARAM_CLUMPING] * (average_position - pos);
		force += parameters[PARAM_ALIGNING] * (average_velocity - p.velocity);

		p.velocity += p_delta * force;


		float v = p.velocity.length();
		v -= p_delta * parameters[PARAM_DAMPING];
		v = v < 0.0 ? 0.0 : v;
		p.velocity = p.velocity.normalized() * v;

		// TODO fix animation
		float animation_phase = 0.0;
		p.custom[2] = animation_phase;

		if (v > 0.0) {
			p.transform.elements[0] = p.velocity / v;
			p.transform.elements[1] = p.transform.elements[0].tangent();
		}

		//scale by scale
		float base_scale = parameters[PARAM_SCALE];
		if (base_scale < 0.000001) base_scale = 0.000001;

		p.transform.elements[0] *= base_scale;
		p.transform.elements[1] *= base_scale;

		p.transform[2] += p_delta * p.velocity;

		// some clamping with velocity corection afterwards

		auto position = p.transform[2];

		auto clamped_position = clamp(position, get_viewport_rect().position, get_viewport_rect().size);

		p.velocity += 2.0 * (clamped_position - position) / p_delta;
		p.transform[2] = clamped_position;

	}
}

void BoidParticles::_update_particle_data_buffer() {
#ifndef NO_THREADS
	update_mutex->lock();
#endif

	{

		int pc = particles.size();

		PoolVector<float>::Write w = particle_data.write();
		PoolVector<Particle>::Read r = particles.read();
		float *ptr = w.ptr();

		for (int i = 0; i < pc; i++) {

			Transform2D t = r[i].transform;

			if (r[i].active) {

				ptr[0] = t.elements[0][0];
				ptr[1] = t.elements[1][0];
				ptr[2] = 0;
				ptr[3] = t.elements[2][0];
				ptr[4] = t.elements[0][1];
				ptr[5] = t.elements[1][1];
				ptr[6] = 0;
				ptr[7] = t.elements[2][1];

				Color c = r[i].color;
				uint8_t *data8 = (uint8_t *)&ptr[8];
				data8[0] = CLAMP(c.r * 255.0, 0, 255);
				data8[1] = CLAMP(c.g * 255.0, 0, 255);
				data8[2] = CLAMP(c.b * 255.0, 0, 255);
				data8[3] = CLAMP(c.a * 255.0, 0, 255);

				ptr[9] = r[i].custom[0];
				ptr[10] = r[i].custom[1];
				ptr[11] = r[i].custom[2];
				ptr[12] = r[i].custom[3];

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

void BoidParticles::_set_redraw(bool p_redraw) {
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

void BoidParticles::_update_render_thread() {

#ifndef NO_THREADS
	update_mutex->lock();
#endif

	VS::get_singleton()->multimesh_set_as_bulk_array(multimesh, particle_data);

#ifndef NO_THREADS
	update_mutex->unlock();
#endif
}

void BoidParticles::_notification(int p_what) {

	if (p_what == NOTIFICATION_ENTER_TREE) {
		_set_redraw(true);
	}

	if (p_what == NOTIFICATION_EXIT_TREE) {
		_set_redraw(false);
	}

	if (p_what == NOTIFICATION_PHYSICS_PROCESS) {

		std::cout << "todo call physics" << std::endl;

	}

	if (p_what == NOTIFICATION_DRAW) {

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
}

void BoidParticles::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_amount", "amount"), &BoidParticles::set_amount);
	ClassDB::bind_method(D_METHOD("get_amount"), &BoidParticles::get_amount);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &BoidParticles::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &BoidParticles::get_texture);

	ClassDB::bind_method(D_METHOD("set_normalmap", "normalmap"), &BoidParticles::set_normalmap);
	ClassDB::bind_method(D_METHOD("get_normalmap"), &BoidParticles::get_normalmap);

	ClassDB::bind_method(D_METHOD("restart"), &BoidParticles::restart);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "amount", PROPERTY_HINT_EXP_RANGE, "1,1000000,1"), "set_amount", "get_amount");

	ADD_GROUP("Drawing", "");
	// No visibility_rect property contrarily to Particles2D, it's updated automatically.
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normalmap", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_normalmap", "get_normalmap");

	////////////////////////////////

	ClassDB::bind_method(D_METHOD("set_param", "param", "value"), &BoidParticles::set_param);
	ClassDB::bind_method(D_METHOD("get_param", "param"), &BoidParticles::get_param);

	ClassDB::bind_method(D_METHOD("set_param_randomness", "param", "randomness"), &BoidParticles::set_param_randomness);
	ClassDB::bind_method(D_METHOD("get_param_randomness", "param"), &BoidParticles::get_param_randomness);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &BoidParticles::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &BoidParticles::get_color);

	ClassDB::bind_method(D_METHOD("set_particle_flag", "flag", "enable"), &BoidParticles::set_particle_flag);
	ClassDB::bind_method(D_METHOD("get_particle_flag", "flag"), &BoidParticles::get_particle_flag);

	ClassDB::bind_method(D_METHOD("_update_render_thread"), &BoidParticles::_update_render_thread);
	ClassDB::bind_method(D_METHOD("_physics_process"), &BoidParticles::_physics_process);
	ClassDB::bind_method(D_METHOD("_texture_changed"), &BoidParticles::_texture_changed);

	auto pn = parameter_names();
	ADD_GROUP("Parameters", "para_");
	for (int pi = 0; pi < PARAM_MAX; pi++) {

		ADD_PROPERTYI(PropertyInfo(Variant::REAL, pn[pi]), "set_param", "get_param", pi);
		ADD_PROPERTYI(PropertyInfo(Variant::REAL, pn[pi] + "_random", PROPERTY_HINT_RANGE, "0, 1, 0.01"), "set_param_randomness", "get_param_randomness", pi);

		ClassDB::bind_integer_constant(get_class_static(), StringName(), pn[pi], pi);
	}

	auto fn = flag_names();
	ADD_GROUP("Flags", "flag_");
	for (int fi = 0; fi < FLAG_MAX; fi++) {

		ADD_PROPERTYI(PropertyInfo(Variant::INT, fn[fi]), "set_flag", "get_flag", fi);

		ClassDB::bind_integer_constant(get_class_static(), StringName(), fn[fi], fi);
	}

	ADD_GROUP("Color", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

}

BoidParticles::BoidParticles() {

	time = 0;
	redraw = false;

	mesh = VisualServer::get_singleton()->mesh_create();
	multimesh = VisualServer::get_singleton()->multimesh_create();
	VisualServer::get_singleton()->multimesh_set_mesh(multimesh, mesh);

	set_amount(8);

	// TODO initialize parameters

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

BoidParticles::~BoidParticles() {
	VS::get_singleton()->free(multimesh);
	VS::get_singleton()->free(mesh);

#ifndef NO_THREADS
	memdelete(update_mutex);
#endif
}
