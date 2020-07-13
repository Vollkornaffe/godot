/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "passive_particles_2d.h"

void register_passive_particles_types() {
    ClassDB::register_class<PassiveParticles2D>();
}

void unregister_passive_particles_types() {
   // Nothing to do here in this example.
}
