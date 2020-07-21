#!/bin/bash

scons platform=x11 use_llvm=yes use_lld=yes -j4
scons platform=x11 use_llvm=yes use_lld=yes tools=no target=release -j4
scons platform=x11 use_llvm=yes use_lld=yes tools=no target=release_debug -j4

echo ""
echo ""
echo ""
echo "    DON'T FORGET TO GENERATE NEW api.json !!!"
echo "    DON'T FORGET TO GENERATE NEW api.json !!!"
echo "    DON'T FORGET TO GENERATE NEW api.json !!!"
