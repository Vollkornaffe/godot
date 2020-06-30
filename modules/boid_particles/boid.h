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

			p.angle_rand = Math::randf();
			p.scale_rand = Math::randf();
			p.hue_rot_rand = Math::randf();
			p.anim_offset_rand = Math::randf();


            p.velocity = Vector2();
            p.rotation = 0.0;

			p.custom[0] = 0.0; // unused
			p.custom[1] = 0.0; // phase [0..1]
			p.custom[2] = 0.0;
			p.custom[3] = 0.0;
			p.transform = Transform2D();
			p.time = 0;
            p.lifetime = 10000.0;
			p.base_color = Color(1, 1, 1, 1);
            p.color = Color(1,1,1,1);

            p.transform.elements[0] *= 10.0;
            p.transform.elements[1] *= 10.0;
            p.transform[2] = Vector2(i * 100, 0);

        }

    }

    void step(Object * p_object, float delta) {

        PassiveParticles * p_particles = Object::cast_to<PassiveParticles>(p_object);
        int n = p_particles->get_amount();
        PoolVector<Particle>::Write w = p_particles->get_particles_write();
        Particle *parray = w.ptr();
        for (int i = 0; i < n; i++) {

            auto &p = parray[i];

            p.transform[2][0] = std::cos(i + t) * 100.0;
            p.transform[2][1] = std::sin(i + t) * 100.0;

        }

        t += delta;

    }

private:

    float t = 0.0;

};

#endif //BOID_NODE_H