#pragma once

#include <cassert>
#include <iostream>
#include "particle.h"
#include "core/math/math_defs.h"

struct BoidSystem {

    float detection_range;
    float detection_angle;
    float avoiding;
    float aligning;
    float clumping;

    void update_particles(PoolVector<Particle>& particles, float p_delta) {

        int pcount = particles.size();
        PoolVector<Particle>::Write w = particles.write();

        Particle *parray = w.ptr();

        for (int i = 0; i < pcount; i++) {

            Particle &p_i = parray[i];

            if (!p_i.active) 
                continue;

            auto position = p_i.transform[2]; // for clumping
            auto velocity = p_i.velocity; // for aligning
            auto normalization = 1.0; // to normalize the above

            for (int j = 0; j < pcount; j++) {

                if (i == j)
                    continue;

                Particle &p_j = parray[j];

                if (!p_j.active) 
                    continue;

                // from j to i
                auto dir = p_i.transform[2] - p_j.transform[2];
                auto sqr_dist = dir.length_squared();

                if (sqr_dist > detection_range * detection_range)
                    continue; // too far

                // velocity is also the orietation
                auto angle = p_i.velocity.angle_to(dir);

                if (abs(angle) > detection_angle * Math_PI / 180.0 / 2.0) 
                    continue; // no in view

                // can see, now compute the actual distance
                auto dist = sqrt(sqr_dist);

                // these go from 1.0 (close / in front) to zero (far / just in sight)
                auto factor_dist = 1.0 - dist / detection_range;
                auto factor_angle = 1.0 - angle / detection_angle / 2.0;
                
                auto factor = factor_dist * factor_dist * factor_angle;
                //auto factor = factor_dist * factor_dist;

                assert(factor >= 0.0);

                position += factor * p_j.transform[2];
                velocity += factor * p_j.velocity;
                normalization += factor;

                //position += p_j.transform[2];
                //velocity += p_j.velocity;
                //normalization += 1.0;

                // this is the avoidance part
                p_i.velocity += p_delta * avoiding * factor * dir;

            }

            position /= normalization;
            velocity /= normalization;

            // this is the clumping & aligning part
            p_i.velocity += p_delta * clumping * (position - p_i.transform[2]);
            p_i.velocity += p_delta * aligning * (velocity - p_i.velocity);
        }

    }

};