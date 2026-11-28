// ASTERIA (Gamebuino AKA) - Carte du monde (menu pause > Carte du monde).
//
// Les zones decouvertes sont nommees et reliees par leurs chemins ; les autres restent
// des « ? » (toutes nommees avec la Carte regionale de la Marche occidentale). Un
// repere rouge montre la position du joueur et une etoile doree le lieu du prochain
// objectif de la quete suivie. Fond : /sdcard/ASTERIA/CARTE.BMP (320x240) s'il existe,
// sinon un parchemin dessine (relief, foret, riviere).
#pragma once
#include <cstdint>
namespace asteria { namespace worldmap {

void open(int quest);   // quete suivie (son prochain objectif est marque), -1 : aucune
bool is_open();
void close();
void update(uint32_t pressed);
void render();

}}
