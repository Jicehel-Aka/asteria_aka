#include "player.h"
#include <cstring>
#include "generated/asteria_gen.h"
#include "generated/asteria_spells_gen.h"
#include "generated/asteria_recipes_gen.h"
namespace asteria {
using namespace data;
int item_index(const char* id){ for(int i=0;i<ITEM_COUNT;++i) if(!strcmp(ITEMS[i].id,id)) return i; return -1; }
int recipe_index(const char* id){ for(int i=0;i<RECIPE_COUNT;++i) if(!strcmp(RECIPES[i].id,id)) return i; return -1; }

int Inventory::find(int item) const { for(int i=0;i<n;++i) if(slots[i].item==item) return i; return -1; }
bool Inventory::has(int item,int qty) const { int i=find(item); return i>=0 && slots[i].qty>=qty; }
bool Inventory::add(int item,int qty){
  int i=find(item);
  if(i>=0){ slots[i].qty+=qty; return true; }
  if(n>=CAP) return false;
  slots[n++]={(int16_t)item,(uint8_t)qty}; return true;
}
bool Inventory::remove(int item,int qty){
  int i=find(item); if(i<0||slots[i].qty<qty) return false;
  slots[i].qty-=qty;
  if(slots[i].qty==0){ slots[i]=slots[--n]; }
  return true;
}

Character new_caithness(){
  Character c{}; c.xp=0; c.gold=20; c.level=1; c.inv.add(4,2);
  c.IN=11; c.CO=9; c.MA=14; c.AU=16;   // valeurs de départ (feuille d'origine)
  c.VIT=20; c.BL=0;                     // VIT de départ : valeur à équilibrer
  c.PO=0; for(int i=0;i<4;++i) c.attitude[i]=0;
  c.weapon=item_index("OBJ_0037");     // Dague
  // sorts de base connus au départ
  c.spells=0;
  for(int i=0;i<SPELL_COUNT;++i) if(SPELLS[i].tier==ST_BASE) c.spells|=(1u<<i);
  c.manaMax=6+c.MA/2; c.mana=c.manaMax;
  return c;
}
void add_attitude(Character& c,int a){ if(a<0||a>3) return; c.attitude[a]++; c.PO++; }

bool try_craft(Character& c,int r,bool at_forge){
  if(r<0||r>=RECIPE_COUNT) return false;
  const Recipe& rc=RECIPES[r];
  if(rc.station==0 && !at_forge) return false;         // 0 = forge requise
  for(int i=0;i<rc.input_n;++i) if(!c.inv.has(rc.inputs[i].item,rc.inputs[i].qty)) return false;
  for(int i=0;i<rc.input_n;++i) c.inv.remove(rc.inputs[i].item,rc.inputs[i].qty);
  c.inv.add(rc.out_item,rc.out_qty);
  return true;
}
}

namespace asteria { Character& player(){ static Character c = new_caithness(); return c; } }
