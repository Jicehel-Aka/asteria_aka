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
// Emplacements d'equipement (v1.5). La main droite tient l'arme : sa valeur est
// Character::weapon (equip[EQ_HAND_R] n'est pas utilise) ; les autres emplacements
// contiennent un index d'objet ou -1. Un objet equipe reste dans le sac.
enum EqSlot : uint8_t {
  EQ_HAND_R, EQ_HAND_L, EQ_RING_R1, EQ_RING_R2, EQ_RING_L1, EQ_RING_L2,
  EQ_NECK, EQ_BODY, EQ_LEGS, EQ_FEET, EQ_HEAD, EQ_COUNT
};
struct Character {
  // caractéristiques de départ (3..30)
  int IN, CO, MA, AU;
  int VIT, BL;   // vitalité, blessures (mort si BL>VIT)
  int PO;                   // pouvoir = somme des attitudes
  int attitude[4];          // 0 Agressive, 1 Amicale, 2 Rusee, 3 Prudente
  int weapon;               // index objet arme equipee
  uint32_t spells;          // masque de bits sur data::SPELLS
  int xp, gold, level;   // progression
  int mana, manaMax;   // pouvoir (mana) pour les sorts
  int torchFuel;            // lumiere dans la Vieille Mine (0..100)
  char name[16];   // DELTA-02 : nom affiche (Caithness, ou nom du voyageur cree)
  bool female;   // DELTA-02 : accords des dialogues (voyageuse / voyageur)
  Inventory inv;
  int16_t equip[EQ_COUNT];  // objets portes (voir EqSlot)
};
int  item_index(const char* id);   // résout un OBJ_xxxx -> index (ou -1)
int  recipe_index(const char* id);
Character new_caithness();
// DELTA-02 : adapte un texte AFFICHE au heros courant (deja traduit par trc()) :
// "Caithness" -> nom du joueur, et accords au masculin (voyageuse -> voyageur,
// viajera -> viajero, Reisende -> Reisender, viaggiatrice -> viaggiatore...).
// Renvoie s tel quel pour Caithness ; sinon un tampon interne (2 en rotation).
const char* hero_text(const char* s);
Character& player();   // crée la feuille de départ
void add_attitude(Character& c, int a);   // +1 attitude -> +1 Pouvoir
bool try_craft(Character& c, int recipe_idx, bool at_forge);

// Equipement (v1.5)
// Bonus cumules de l'equipement porte : armure (degats subis en moins), CO (toucher),
// degats, AU (prix, coups critiques).
struct EqStats { int arm, co, dmg, hit, au; };
int  item_eq_slot(int item);                 // emplacement naturel (anneau : EQ_RING_R1), ou -1
bool is_equippable(int item);
bool is_two_handed(int item);                // arcs : occupent aussi la main gauche
int  eq_get(const Character& c, int slot);
bool is_equipped(const Character& c, int item);
// Equipe item a l'emplacement slot (-1 : emplacement naturel, premier anneau libre).
// Renvoie un message deja traduit (dans out) ; false si impossible.
bool equip_item(Character& c, int item, int slot, char* out, int cap);
bool unequip_slot(Character& c, int slot, char* out, int cap);
EqStats eq_stats(const Character& c);
// Bonus qu'aurait le personnage avec item equipe a slot (comparaison avant d'equiper)
EqStats eq_stats_if(const Character& c, int item, int slot);
const char* eq_slot_name(int slot);          // FR, a passer par trc()
bool slot_accepts(int slot, int item);
void eq_clear(Character& c);
void eq_auto_legacy(Character& c);           // vieilles sauvegardes : porte ce qui etait passif

// Utilitaires RPG (artisanat, armures, boutique, consommables, lumiere)
int  armor_defense(const Character& c);
int  weapon_bonus_of(int item, int& hitBonus, bool& isBow);
bool is_weapon_item(int item);
bool is_consumable_field(int item);
bool use_item_field(Character& c, int item, char* out, int cap);
const char* item_desc(int item);
int  item_base_price(int item);
int  apply_au_discount(int price, int au);
int  item_sell_price(int item);
bool is_item_sellable(int item);
}
