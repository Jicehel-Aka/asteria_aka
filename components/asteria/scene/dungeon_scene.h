#pragma once
#include "scene/scene.h"
#include <cstdint>
namespace asteria {
// Donjon en vue premiere personne (pseudo-3D grille facon Dark & Under),
// avec panneau d'info a droite (boussole, auto-carte, PV/Mana/Or, journal).
struct DungeonScene : Scene {
  static const int GW=15, GH=15;
  uint8_t  cell[GW*GH];   // 0 sol, 1 mur, 2 filon (mur), 5 eau (sol)
  uint8_t  seen[GW*GH];   // auto-carte : 1 si deja vu
  int px=1, py=1, face=2; // 0=N 1=E 2=S 3=O (grille / logique)
  int bx=0, by=0;         // salle du boss / sortie vers Grand-Castel
  int enx=1, eny=1;       // entree (sortie vers la foret)
  bool gen=false; int steps=0; uint32_t rng=1;
  char jr[4][40]; int jrn=0;
  // pose de rendu continue + animation de pas/rotation
  float vx=1.5f, vy=1.5f, vdx=0.f, vdy=1.f;
  bool anim=false; int animKind=0; float at=0.f;
  float afx=0,afy=0,atx=0,aty=0, afdx=0,afdy=0,atdx=0,atdy=0; bool pendEnc=false;
  void enter() override; void update(SceneManager&) override; void render() override;
  void generate(); void journal(const char* s); void mark_seen(); uint32_t rnd();
};
DungeonScene& dungeon_scene();
void dungeon_reset();   // force une nouvelle generation (nouvelle partie)
}
