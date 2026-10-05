#pragma once
#include "scene/scene.h"
namespace asteria {
void start_combat(int monsterIndex, const char* backdropBmp);
struct CombatScene : Scene {
  int phase=0, sel=0, result=0; unsigned mhp=0; int lastDmg=0; int spellSel=0; bool stunned=false; char msg[64];
  void enter() override; void update(SceneManager&) override; void render() override;
};
CombatScene& combat_scene();
}
