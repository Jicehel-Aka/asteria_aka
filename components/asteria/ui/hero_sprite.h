// ASTERIA (Gamebuino AKA) - Sprite du heros selon le personnage : Caithness (ou une
// voyageuse creee) -> spr::HERO ; voyageur cree au masculin -> spr::HERO_M (meme format).
#pragma once
#include "generated/asteria_hero.h"
#include "generated/asteria_hero_m.h"
#include "player.h"
namespace asteria {
inline const spr::Sprite& hero_sprite(int dir, int frame){
  return player().female ? spr::HERO[dir][frame] : spr::HERO_M[dir][frame];
}
}
