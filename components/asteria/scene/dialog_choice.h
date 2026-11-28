// ASTERIA (Gamebuino AKA) - Dialogues a choix, sur le modele du livre-jeu « Defis et
// Sortileges » : a certains moments cles, le heros choisit une ATTITUDE (Agressive,
// Amicale, Rusee, Prudente). Chaque reponse donne +1 dans cette attitude (+1 Pouvoir,
// voir add_attitude) et peut avoir un effet : or gagne ou paye, objet obtenu, indice.
//
// Un jeu de choix est attache a un noeud de dialogue (PNJ + noeud) : quand ses lignes
// sont lues, la scene du monde appelle begin() ; la reponse du PNJ s'affiche ensuite,
// puis les effets du noeud (drapeau, objet, quete) s'appliquent si le choix les accepte.
// Chaque jeu n'est propose qu'une fois (drapeau "ch_...").
#pragma once
#include <cstdint>
namespace asteria { namespace choice {

enum Result { RUNNING, DONE_ACCEPT, DONE_DECLINE };

bool   begin(int npc, int node);       // true si un choix s'ouvre pour ce noeud
bool   active();
Result update(uint32_t pressed);       // DONE_ACCEPT : appliquer les effets du noeud
void   render(const char* npcName);    // remplace la boite de dialogue tant qu'actif
void   reset();

}}
