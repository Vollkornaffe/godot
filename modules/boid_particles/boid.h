#ifndef BOID_NODE_H
#define BOID_NODE_H

#include "core/reference.h"
#include "passive_particles.h"
#include "particle.h"

class Boid : public Reference {
    GDCLASS(Boid, Reference);

protected:
    static void _bind_methods() {

        ClassDB::bind_method(D_METHOD("init_particles"), &Boid::init_particles);
        ClassDB::bind_method(D_METHOD("step"), &Boid::step);

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
        Ref<Texture> sdf_texture,
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

            p.velocity -= delta * p.transform[2];
            p.transform[2] += delta * p.velocity;

            auto vn = p.velocity.length();

            p.transform.elements[1] = -p.velocity / (vn == 0 ? 1.0 : vn);
            p.transform.elements[0] = p.transform.elements[1].tangent();

            p.custom[2] = fmod(p.time, 1.0/3.0) + 1.0/3.0;

            p.time += vn / 1000.0 * animation_speed;

        }

        t += delta;

    }

private:

    float t = 0.0;

};

#endif //BOID_NODE_H