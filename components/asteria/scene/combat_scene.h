#pragma once
#include "scene/scene.h"
namespace asteria {
void start_combat(int monsterIndex, const char* backdropBmp);
void set_combat_return(SceneId s);   // scene de retour apres victoire/fuite (defaut WORLD)
struct CombatScene : Scene {
  int phase=0, sel=0, result=0; unsigned mhp=0; int lastDmg=0; int spellSel=0; bool stunned=false; char msg[64];
  // effets de sorts en cours (BUG-02) : tours restants
  int hitBuff=0, hitBonus=0, dmgBonus=0, summon=0, bear=0, shield=0, invis=0, enemyDebuff=0; bool haste=false; bool summonActed=false;
  void cast(int spell);
  void enter() override; void update(SceneManager&) override; void render() override;
};
CombatScene& combat_scene();
bool spell_known(int i);          // sort data::SPELLS[i] utilisable par le joueur
int  spell_cost(int i);           // cout en pouvoir
const char* spell_short(int i);   // nom court (FR, passe par trc())
}
