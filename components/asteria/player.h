#pragma once
#include <cstdint>
namespace asteria {
struct ItemStack { int16_t item; uint8_t qty; };
struct Inventory {
  static const int CAP = 40;
  ItemStack slots[CAP]; uint8_t n = 0;
  int  find(int item) const;
  bool has(int item, int qty) const;
  bool add(int item, int qty);
  bool remove(int item, int qty);
};
struct Character {
  // caractéristiques de départ (3..30)
  int IN, CO, MA, AU;
  int VIT, BL;              // vitalité, blessures (mort si BL>VIT)
  int PO;                   // pouvoir = somme des attitudes
  int attitude[4];          // 0 Agressive, 1 Amicale, 2 Rusée, 3 Prudente
  int weapon;               // index objet arme équipée
  uint32_t spells;          // masque de bits sur data::SPELLS
  int xp, gold, level;       // progression
  int mana, manaMax;         // pouvoir (mana) pour les sorts
  char name[16];             // DELTA-02 : nom affiche (Caithness, ou nom du voyageur cree)
  bool female;               // DELTA-02 : accords des dialogues (voyageuse / voyageur)
  Inventory inv;
};
int  item_index(const char* id);      // résout un OBJ_xxxx -> index (ou -1)
int  recipe_index(const char* id);
Character new_caithness();
// DELTA-02 : adapte un texte AFFICHE au heros courant (deja traduit par trc()) :
// "Caithness" -> nom du joueur, et accords au masculin (voyageuse -> voyageur,
// viajera -> viajero, Reisende -> Reisender, viaggiatrice -> viaggiatore...).
// Renvoie s tel quel pour Caithness ; sinon un tampon interne (2 en rotation).
const char* hero_text(const char* s);
Character& player();             // crée la feuille de départ
void add_attitude(Character& c, int a);// +1 attitude -> +1 Pouvoir
bool try_craft(Character& c, int recipe_idx, bool at_forge);
}
