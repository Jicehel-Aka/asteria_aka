// ASTERIA (Gamebuino AKA) - Musique d'ambiance.
//
// Les morceaux sont des fichiers PMF sur la carte SD (/sdcard/ASTERIA/music/*.PMF),
// composes par tools/compose_music.py. Ils sont charges a la demande dans un des
// DEUX tampons (PSRAM sur AKA) et joues en boucle par le lecteur PMF de la
// bibliotheque (gb::music_play). On peut remplacer un morceau par n'importe quel
// .xm/.mod/.s3m/.it converti avec pmf_converter, sans recompiler.
//
// update() choisit le theme selon la scene, le lieu et l'heure, et ne recharge
// rien tant que le theme ne change pas. Un jingle (victoire, mort, niveau) coupe
// la musique jusqu'au prochain changement de scene.
#pragma once
#include "scene/scene.h"
namespace asteria { namespace music {

enum Theme { NONE = 0, TITLE, VILLAGE, NIGHT, FOREST, MINE, ROAD, COMBAT, TAVERN, THEME_COUNT };

void  update(SceneId scene);          // chaque image (asteria_app)
void  play(Theme t);                  // force un theme (rien si deja en cours)
void  stop();
void  hold_for_jingle();              // coupe la musique pendant un jingle, jusqu'au prochain changement de scene
Theme current();
const char* file_of(Theme t);         // chemin SD du morceau

}}
