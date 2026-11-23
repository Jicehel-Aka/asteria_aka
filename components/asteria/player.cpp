// ASTERIA (Gamebuino AKA) - Personnage joueur : caracteristiques (IN/CO/MA/AU/VIT...), inventaire (saturation 255), et fabrication try_craft().
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
  if(qty<=0) return true;
  int i=find(item);
  if(i>=0){ int nq=slots[i].qty+qty; slots[i].qty=(uint8_t)(nq>255?255:nq); return true; }  // BUG-07 : saturation
  if(n>=CAP) return false;
  slots[n++]={(int16_t)item,(uint8_t)(qty>255?255:qty)}; return true;
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
  strcpy(c.name,"Caithness"); c.female=true;   // heroine prétiree (Bible)
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

namespace asteria { Character& player(){ static Character c = new_caithness(); return c; } 
// --- DELTA-02 : nom et accords du heros dans les textes affiches -------------
// Remplace toutes les occurrences de 'from' par 'to' dans dst (taille cap).
static void repl_all(char* dst,int cap,const char* from,const char* to){
  static char tmp[256]; int fl=(int)strlen(from), tl=(int)strlen(to), o=0;
  for(const char* p=dst; *p && o<(int)sizeof(tmp)-1; ){
    if(strncmp(p,from,fl)==0){ for(int k=0;k<tl && o<(int)sizeof(tmp)-1;k++) tmp[o++]=to[k]; p+=fl; }
    else tmp[o++]=*p++; }
  tmp[o]=0; strncpy(dst,tmp,cap-1); dst[cap-1]=0;
}
const char* hero_text(const char* s){
  const Character& c=player();
  if(!s) return "";
  bool rename=strcmp(c.name,"Caithness")!=0;
  if(c.female && !rename) return s;                       // cas par defaut : aucun travail
  static char buf[2][256]; static int k=0; char* b=buf[k^=1];
  strncpy(b,s,255); b[255]=0;
  if(rename) repl_all(b,256,"Caithness",c.name);
  if(!c.female){
    // ordre : formes longues (avec article) d'abord
    static const char* const M[][2]={
      {"la voyageuse","le voyageur"},{"voyageuse","voyageur"},                       // FR
      {"la viajera","el viajero"},{"viajera","viajero"},{"Bienvenida","Bienvenido"},  // ES
      {"die Reisende","der Reisende"},{", Reisende",", Reisender"},                   // DE
      {"la viaggiatrice","il viaggiatore"},{"viaggiatrice","viaggiatore"},{"Benvenuta","Benvenuto"},  // IT
      {"fille des fees","fils des fees"},{"fae daughter","fae son"},{"hija de las hadas","hijo de las hadas"},
      {"Feentochter","Feensohn"},{"figlia delle fate","figlio delle fate"} };       // "enfant des fees" (Orwenn, Voyageur)
    for(const auto& r:M) repl_all(b,256,r[0],r[1]);
  }
  return b;
}
}
