// ASTERIA (Gamebuino AKA) - Icones d'objets et fenetre "Objet obtenu".
//
// Les icones (24x24 et 48x48) sont generees par tools/bake_icons.py dans
// generated/asteria_icons.h ; seul item_ui.cpp inclut ce gros en-tete.
//
// Fenetre "Objet obtenu" : chaque gain (cueillette, coffre, don d'un PNJ) est mis en
// file par pickup_push() ; la scene du monde affiche la file une fenetre a la fois
// (icone, nom, quantite, description) et A ou B passe a la suivante.
#pragma once
#include <cstdint>
namespace asteria { namespace item_ui {

const int GOLD = -2;   // pseudo-objet "bourse d'or" pour pickup_push / draw_icon

// Dessine l'icone d'un objet (index data::ITEMS, ou GOLD) en 24x24 ou 48x48.
void draw_icon(int item, int x, int y, bool big);

// Poupee d'equipement illustree (tools/icon_src/poupee.png), centree en cx a partir de
// top. Renvoie false si aucune illustration n'a ete fournie (silhouette dessinee alors).
bool draw_doll(int cx, int top, int h);

void pickup_push(int item, int qty);   // met un gain en file (item = GOLD : qty pieces d'or)
bool pickup_active();                  // une fenetre est a l'ecran
void pickup_update(uint32_t pressed);  // A / B : fenetre suivante
void pickup_render();
void pickup_clear();                   // nouvelle partie / chargement

// Coupe s en lignes d'au plus maxw pixels (police corps), aux espaces. Renvoie le
// nombre de lignes ecrites dans out (au plus nmax, la derniere est tronquee).
int wrap_text(const char* s, int maxw, char out[][64], int nmax);

}}
