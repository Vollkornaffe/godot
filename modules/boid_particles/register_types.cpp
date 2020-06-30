/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "sdf_texture_rect.h"
#include "boid.h"
#include "passive_particles.h"
#include "sdf.h"

void register_boid_particles_types() {
    ClassDB::register_class<SDFTextureRect>();
    ClassDB::register_class<Boid>();
    ClassDB::register_class<PassiveParticles>();
    ClassDB::register_class<SDF>();
}

void unregister_boid_particles_types() {
   // Nothing to do here in this example.
}
