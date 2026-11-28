// Helpers de progression (XP de quête, montée de niveau, bonus de découverte,
// verrous). Partagés entre world_scene et combat_scene.
#pragma once
#include <cstdint>
namespace asteria {
// Crédite de l'XP, applique les montées de niveau (+5 VIT/niveau). Renvoie true
// si au moins un niveau a été gagné.
bool gain_xp(int xp);
// Termine une quête UNE fois : quest_complete + XP de la quête + jingle
// (level-up si montée, victoire sinon). Sans effet si déjà terminée.
void complete_quest_reward(int qi);
// Bonus de Découverte : +50 XP la première fois qu'on entre dans une zone (loc).
void discover_zone(int loc);
bool zone_discovered(int loc);
// Masque des zones decouvertes (sauvegarde : ligne D) - 64 zones max.
uint64_t discovered_mask();
void set_discovered_mask(uint64_t m);
// Remet à zéro l'état de progression local (découvertes). Appelé par new_game_reset.
void progress_reset();
}
