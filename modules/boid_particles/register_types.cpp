/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "boid_particles.h"

void register_boid_particles_types() {
    ClassDB::register_class<BoidParticles>();
}

void unregister_boid_particles_types() {
   // Nothing to do here in this example.
}
