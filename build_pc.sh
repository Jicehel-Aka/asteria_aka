#!/usr/bin/env bash
set -e
A=components/asteria
SRC="$A/asteria_app.cpp $A/ui/text.cpp $A/player.cpp $A/scene/scene.cpp $A/scene/title_scene.cpp $A/scene/create_scene.cpp $A/scene/rules_scene.cpp $A/scene/world_scene.cpp"
g++ -std=c++17 -I "$A" $SRC gb_port_host.cpp main_host.cpp -o asteria_host && echo "-> ./asteria_host"
command -v sdl2-config >/dev/null 2>&1 && g++ -std=c++17 -I "$A" $SRC gb_port_sdl.cpp main_sdl.cpp $(sdl2-config --cflags --libs) -o asteria_pc && echo "-> ./asteria_pc" || echo "(SDL2 absent)"
