// ASTERIA (Gamebuino AKA) - Horloge du jeu : cycle jour / nuit.
//
// 1 seconde reelle = 1 minute de jeu (une journee complete = 24 min), et le temps
// n'avance qu'en exploration (monde, mine) : un combat ou un menu ne fait pas
// tomber la nuit. Phases : nuit 21h-5h, aube 5h-7h, jour 7h-18h, crepuscule 18h-21h.
// L'heure est sauvegardee (ligne T de SAVE.DAT) ; dormir a l'auberge mene a 8h.
#pragma once
#include <cstdint>
namespace asteria { namespace daytime {

void     reset(int minutes = 8 * 60);   // nouvelle partie : 8h du matin
void     tick(uint32_t now_ms);         // a chaque image d'exploration
int      minutes();                     // 0..1439
void     set_minutes(int m);
// Lumiere ambiante exterieure 0..256 (256 = plein jour). Transitions douces a l'aube
// et au crepuscule ; la nuit ne descend pas sous ~30 % pour rester jouable.
int      ambient();
bool     is_night();                    // vrai de 20h a 6h (musique, rencontres, fonds de combat)

}}
