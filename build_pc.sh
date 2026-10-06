#!/usr/bin/env bash
# Build PC d'ASTERIA — lancer depuis la racine asteria_aka/.
set -e
A=components/asteria
SRC="$A/asteria_app.cpp $A/ui/text.cpp $A/player.cpp $A/gamestate.cpp $A/i18n.cpp $A/autotile.cpp $A/audio/jingle.cpp $A/progress.cpp $A/scene/*.cpp"
echo "[1/2] version captures (sans dependance)"
g++ -std=c++17 -I "$A" $SRC gb_port_host.cpp main_host.cpp -o asteria_host && echo "   -> ./asteria_host"
echo "[2/2] version SDL (fenetre + clavier)"
if command -v sdl2-config >/dev/null 2>&1; then
  g++ -std=c++17 -I "$A" $SRC gb_port_sdl.cpp main_sdl.cpp $(sdl2-config --cflags --libs) -o asteria_pc && echo "   -> ./asteria_pc"
else echo "   (SDL2 absent : voir README)"; fi
