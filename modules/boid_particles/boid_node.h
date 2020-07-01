#ifndef BOID_NODE_H
#define BOID_NODE_H

#include "scene/2d/node_2d.h"
#include "passive_particles.h"
#include "particle.h"
#include "sdf.h"
#include "core/variant.h"
#include "core/type_info.h"

class BoidNode : public Node2D {
    GDCLASS(BoidNode, Node2D);

public:

    enum Parameter {
        PARAM_BOUNDARY_RANGE,
        PARAM_BOUNDARY_FORCE,

        PARAM_BOID_DETECTION_RANGE,
        PARAM_BOID_DETECTION_ANGLE,
        PARAM_BOID_AVOIDING,
        PARAM_BOID_ALIGNING,
        PARAM_BOID_CLUMPING,

        PARAM_LINEAR_DRAG,

        PARAM_MAX,
    };


protected:
    static void _bind_methods();

public:


    void set_param(Parameter p_param, float p_value);
    float get_param(Parameter p_param) const;

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
            p.transform[2][0] = random.randf_range(540, 740);
            p.transform[2][1] = random.randf_range(260, 460);

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

            auto &p_i = parray[i];
            if (!p_i.active) continue;

            // put on stack for convinience
            auto position = p_i.transform[2];
            auto velocity = p_i.velocity;
            auto force = Vector2();

            // boundary force & position update if colliding
            auto sdf_idx = p_sdf->idx(position.x, position.y);
            auto sdf_value = p_sdf->values[sdf_idx];
            auto sdf_gradient = p_sdf->gradients[sdf_idx];
            if (sdf_value < 0.0) {
                auto p_delta = abs(sdf_value) * sdf_gradient;
                position += p_delta;
                velocity += p_delta / delta / 2.0;
            } else if (sdf_value < parameters[PARAM_BOUNDARY_RANGE]) {
                auto factor = (parameters[PARAM_BOUNDARY_RANGE] - sdf_value);
                force += delta * parameters[PARAM_BOUNDARY_FORCE] * factor * factor * p_sdf->gradients[sdf_idx];
            }

            // boid forces
            auto boid_position = position; // for clumping
            auto boid_velocity = velocity; // for aligning
            auto boid_normalization = 1.0; // to normalize the above
            for (int j = 0; j < n; j++) {

                auto &p_j = parray[j];
                if (!p_j.active) continue;

                // from i to j
                auto dir = p_j.transform[2] - position;
                auto sqr_dist = dir.length_squared();
                if (sqr_dist > parameters[PARAM_BOID_DETECTION_RANGE]
                             * parameters[PARAM_BOID_DETECTION_RANGE])
                    continue; // too far

                // velocity is also the orietation
                auto angle = velocity.angle_to(dir);

                if (abs(angle) > parameters[PARAM_BOID_DETECTION_ANGLE] * Math_PI / 180.0 / 2.0) 
                    continue; // no in view

                // can see, now compute the actual distance
                auto dist = sqrt(sqr_dist);

                // these go from 1.0 (close / in front) to zero (far / just in sight)
                auto factor_dist = 1.0 - dist / parameters[PARAM_BOID_DETECTION_RANGE];
                auto factor_angle = 1.0 - angle / parameters[PARAM_BOID_DETECTION_ANGLE] / 2.0;

                auto factor = factor_dist * factor_dist * factor_angle;
                //auto factor = factor_dist * factor_dist;

                assert(factor >= 0.0);

                boid_position += factor * p_j.transform[2];
                boid_velocity += factor * p_j.velocity;
                boid_normalization += factor;

                // this is the avoidance part
                force -= parameters[PARAM_BOID_AVOIDING] * factor * dir;

            }

            // update with the calculated force
            velocity += delta * force;
            position += delta * velocity;

            // damping
            velocity -= delta * parameters[PARAM_LINEAR_DRAG] * velocity;

            auto smoothed_velocity = (p_i.velocity + velocity) / 2.0;
            auto vn = smoothed_velocity.length();

            // rotate the sprite to align with move direction
            if (vn > 0.0) {
                p_i.transform.elements[1] = -smoothed_velocity / (vn == 0 ? 1.0 : vn);
                p_i.transform.elements[0] = p_i.transform.elements[1].tangent();
            }

            // set animation phase
            p_i.custom[2] = fmod(p_i.time, 1.0/3.0) + 1.0/3.0;

            // write values
            p_i.velocity = velocity;
            p_i.transform[2] = position;

            // advance time for animation
            p_i.time += vn / 1000.0 * animation_speed;

        }

        t += delta;

    }

private:
    float t = 0.0;
    float parameters[PARAM_MAX];
};

VARIANT_ENUM_CAST(BoidNode::Parameter)

#endif //BOID_NODE_H