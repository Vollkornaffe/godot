#ifndef BOID_NODE_H
#define BOID_NODE_H

#include "scene/2d/node_2d.h"
#include "passive_particles.h"
#include "particle.h"
#include "sdf.h"

class BoidNode : public Node2D {
    GDCLASS(BoidNode, Node2D);

protected:
    static void _bind_methods() {

        ClassDB::bind_method(D_METHOD("init_particles"), &BoidNode::init_particles);
        ClassDB::bind_method(D_METHOD("step"), &BoidNode::step);

    }

public:

    enum Parameter {
        PARAM_MAX,
    };

    void init_particles(Object * p_object) {

        RandomCustom random;

        PassiveParticles * p_particles = Object::cast_to<PassiveParticles>(p_object);
        if (p_particles == NULL) {
            std::cout << "Cast to PassiveParticles failed!" << std::endl;
            return;
        }

        int n = p_particles->get_amount();
        PoolVector<Particle>::Write w = p_particles->get_particles_write();

        Particle *parray = w.ptr();
        for (int i = 0; i < n; i++) {

            auto &p = parray[i];

            p.active = true;

			p.seed = Math::rand();

            p.velocity = Vector2();
			p.custom[0] = 0.0; // unused
			p.custom[1] = 0.0; // phase [0..1]
			p.custom[2] = 0.0;
			p.custom[3] = 0.0;
			p.transform = Transform2D();
			p.time = 0;
            p.color = Color(1,1,1,1);
            p.transform[2][0] = random.randf_range(50.0, 100.0);
            p.transform[2][1] = random.randf_range(50.0, 100.0);


        }

    }

    void step(
        Object * po_particles,
        Ref<SDF> p_sdf,
        float delta
    ) {

        PassiveParticles * p_particles = Object::cast_to<PassiveParticles>(po_particles);

        int n = p_particles->get_amount();
        PoolVector<Particle>::Write w = p_particles->get_particles_write();
        Particle *parray = w.ptr();

        auto animation_speed = p_particles->get_param(PassiveParticles::Parameter::PARAM_ANIM_SPEED);

        for (int i = 0; i < n; i++) {

            /* 
            for (int j = 0; j < n; j++) {
            }
            */

            auto &p = parray[i];

            auto position = p.transform[2];
            auto velocity = p.velocity;
            auto sdf_idx = p_sdf->idx(position.x, position.y);

            auto force = Vector2();

            auto sdf_value = p_sdf->values[sdf_idx];
            auto sdf_gradient = p_sdf->gradients[sdf_idx];
            if (sdf_value < 0.0) {
                auto p_delta = abs(sdf_value) * sdf_gradient;
                position += p_delta;
                velocity += p_delta / delta / 2.0;
            } else if (sdf_value < 100.0) { // TODO make this a parameter
                force += delta * abs(sdf_value) * p_sdf->gradients[sdf_idx];
            }

            velocity += delta * force;
            position += delta * p.velocity;

            auto vn = p.velocity.length();

            if (vn > 0.0) {
                p.transform.elements[1] = -p.velocity / (vn == 0 ? 1.0 : vn);
                p.transform.elements[0] = p.transform.elements[1].tangent();
            }

            p.custom[2] = fmod(p.time, 1.0/3.0) + 1.0/3.0;

            p.velocity = velocity;
            p.transform[2] = position;

            p.time += vn / 1000.0 * animation_speed;

        }

        t += delta;

    }

private:
    float t = 0.0;

};

#endif //BOID_NODE_H