#include "boid_particles.h"

void BoidParticles::add(int p_value) {
    count += p_value;
}

void BoidParticles::reset() {
    count = 0;
}

int BoidParticles::get_total() const {
    return count;
}

void BoidParticles::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add", "value"), &BoidParticles::add);
    ClassDB::bind_method(D_METHOD("reset"), &BoidParticles::reset);
    ClassDB::bind_method(D_METHOD("get_total"), &BoidParticles::get_total);
}

BoidParticles::BoidParticles() {
    count = 0;
}
