#ifndef BOID_PARTICLES_H
#define BOID_PARTICLES_H

#include "core/reference.h"

class BoidParticles : public Reference {
    GDCLASS(BoidParticles, Reference);

    int count;

protected:
    static void _bind_methods();

public:
    void add(int p_value);
    void reset();
    int get_total() const;

    BoidParticles();
};

#endif // BOID_PARTICLES_H
