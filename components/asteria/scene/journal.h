// ASTERIA (Gamebuino AKA) - Journal de quetes (menu pause > Journal de quetes).
//
// Deux onglets (Gauche / Droite) : quetes en cours et quetes terminees. Pour la quete
// selectionnee : type (principale / secondaire), objectifs coches selon l'etat reel
// du jeu (drapeaux, sac, decouvertes), prochaine etape et lieu ou se rendre.
#pragma once
#include <cstdint>
namespace asteria { namespace journal {

void open();
bool is_open();
void close();
void update(uint32_t pressed);
void render();

// Prochain objectif d'une quete en cours (texte FR, a passer par trc) et son lieu
// (identifiant de zone, -1 si inconnu). Sert aussi au bandeau et a la carte du monde.
const char* next_objective(int q, int* loc);

}}
