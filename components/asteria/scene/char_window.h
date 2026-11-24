// ASTERIA (Gamebuino AKA) - Fenetre personnage (ouverte depuis le monde par RUN, ou
// depuis le menu pause, entree "Personnage").
//
// Colonne gauche : portrait, niveau, or, PV / PM, caracteristiques, armure.
// Colonne droite, deux onglets (RUN pour passer de l'un a l'autre) :
//   Sac   : equipement porte (arme, bouclier, casque, cape, anneau) et grille d'icones du
//           sac ; A equipe une arme, utilise un consommable ou decrit l'objet.
//   Sorts : sorts connus et leur cout ; A lance Soins / Guerison majeure hors combat.
// B ferme la fenetre.
#pragma once
#include <cstdint>
namespace asteria { namespace charwin {

void open(int tab = 0);
bool is_open();
void close();
void update(uint32_t pressed);   // a appeler seulement si is_open()
void render();

}}
