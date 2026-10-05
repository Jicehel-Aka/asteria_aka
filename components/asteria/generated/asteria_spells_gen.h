// GENERE PAR tools/bake.py
#pragma once
#include <cstdint>
namespace asteria { namespace data {
enum SpellTier:uint8_t{ ST_BASE=0, ST_ADVANCED=1 };
static constexpr uint8_t SPELL_COUNT=12;
struct Spell{const char* id;const char* name;uint8_t tier;uint8_t level;uint8_t initiative;uint8_t po_cost;const char* effect;};
static const Spell SPELLS[SPELL_COUNT]={
  {"SPL_0001","Attirance",ST_BASE,1,10,0,"+1D Aura pendant 3 tours"},
  {"SPL_0002","Engourdissement",ST_BASE,2,9,0,"-2D Initiative et -1D Combat à l'ennemi"},
  {"SPL_0003","Dissipation de la Magie",ST_BASE,2,8,0,"annule les effets magiques proches"},
  {"SPL_0004","Répulsion",ST_BASE,1,9,0,"-2D Aura à la cible"},
  {"SPL_0005","Soins",ST_BASE,2,7,0,"-2D Blessures"},
  {"SPL_0006","Accélération temporelle",ST_ADVANCED,4,12,6,"agir 2 fois au prochain tour"},
  {"SPL_0007","Appel des Éléments",ST_ADVANCED,4,11,8,"invoque une créature élémentaire (IN 12, CO 14, VIT 20)"},
  {"SPL_0008","Guérison majeure",ST_ADVANCED,3,7,5,"-4D Blessures"},
  {"SPL_0009","Métamorphose",ST_ADVANCED,5,10,10,"transformation en ours (IN 20, CO 16, MA 0, VIT 40, BL 3D)"},
  {"SPL_0010","Invisibilité",ST_ADVANCED,4,10,7,"indétectable un temps"},
  {"SPL_0011","Mimétisme",ST_ADVANCED,3,9,4,"-2D Combat aux ennemis"},
  {"SPL_0012","Vivacité",ST_ADVANCED,3,11,4,"+2D Initiative, +1D Combat"},
};
} }
