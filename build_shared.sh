#!/bin/bash

scons platform=x11 use_llvm=yes use_lld=yes boid_particles_shared=yes bin/libboid_particles.x11.tools.64.llvm.so -j4

# first time around also execute this
#scons platform=x11 use_llvm=yes use_lld=yes boid_particles_shared=yes -j4
