#ifndef BOID_NODE_H
#define BOID_NODE_H

#include "scene/2d/node_2d.h"
#include "passive_particles.h"

class BoidNode : public Node2D {
    GDCLASS(BoidNode, Node2D);

protected:
    static void _bind_methods() {

        ClassDB::bind_method(D_METHOD("test_update"), &BoidNode::test_update);

    }

public:

    enum Parameter {
        PARAM_MAX,
    };

    void test_update(Object * p_object) {

        PassiveParticles * p_particles = Object::cast_to<PassiveParticles>(p_object);
        if (p_particles == NULL) {
            std::cout << "Cast to PassiveParticles failed!" << std::endl;
            return;
        }

        int n = p_particles->get_amount();
        PoolVector<PassiveParticles::Particle>::Write w = p_particles->get_particles_write();

        PassiveParticles::Particle *parray = w.ptr();
        for (int i = 0; i < n; i++) {

            auto &p_i = parray[i];

            p_i.active = true;
            p_i.transform[2] = Vector2(i, 0);

        }

    }

};

#endif //BOID_NODE_H