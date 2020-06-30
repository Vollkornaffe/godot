/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "boid_particles.h"
#include "sdf_texture_rect.h"
#include "boid_node.h"

void register_boid_particles_types() {
    ClassDB::register_class<BoidParticles>();
    ClassDB::register_class<SDFTextureRect>();
    ClassDB::register_class<BoidNode>();
    ClassDB::register_class<PassiveParticles>();
}

void unregister_boid_particles_types() {
   // Nothing to do here in this example.
}
