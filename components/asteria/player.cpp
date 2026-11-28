// ASTERIA (Gamebuino AKA) - Joueur, inventaire, equipement, armures, consommables & artisanat.
#include "player.h"
#include <cstring>
#include <cstdio>
#include "content_i18n.h"
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
  if(item<0 || item>=ITEM_COUNT || qty<=0) return false;
  int i=find(item);
  if(i>=0){ int nq=slots[i].qty+qty; slots[i].qty=(uint8_t)(nq>255?255:nq); return true; }   // BUG-07 : saturation
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
  Character c{}; c.xp=0; c.gold=20; c.level=1;
  c.inv.add(4,2);   // 2x Petite potion de soin
  c.inv.add(2,1);   // 1x Torche de depart pour la mine
  c.IN=11; c.CO=9; c.MA=14; c.AU=16;   // valeurs de départ (feuille d'origine)
  c.VIT=20; c.BL=0;   // VIT de départ : valeur à équilibrer
  c.PO=0; for(int i=0;i<4;++i) c.attitude[i]=0;
  c.weapon=item_index("OBJ_0037");     // Dague
  // sorts de base connus au départ
  c.spells=0;
  for(int i=0;i<SPELL_COUNT;++i) if(SPELLS[i].tier==ST_BASE) c.spells|=(1u<<i);
  c.manaMax=6+c.MA/2; c.mana=c.manaMax;
  c.torchFuel=100;
  strcpy(c.name,"Caithness"); c.female=true;   // heroine prétiree (Bible)
  return c;
}
void add_attitude(Character& c,int a){
  if(a<0||a>3) return;
  c.attitude[a]++; c.PO++;
  if((c.PO%4)==0){ c.manaMax++; c.mana++; }   // les attitudes renforcent le Pouvoir interieur
}
bool try_craft(Character& c,int r,bool at_forge){
  if(r<0||r>=RECIPE_COUNT) return false;
  const Recipe& rc=RECIPES[r];
  if(rc.station==0 && !at_forge) return false;   // 0 = forge requise
  for(int i=0;i<rc.input_n;++i) if(!c.inv.has(rc.inputs[i].item,rc.inputs[i].qty)) return false;
  for(int i=0;i<rc.input_n;++i) c.inv.remove(rc.inputs[i].item,rc.inputs[i].qty);
  c.inv.add(rc.out_item,rc.out_qty);
  return true;
}

int weapon_bonus_of(int item, int& hitBonus, bool& isBow){
  hitBonus=0; isBow=false;
  if(item<0||item>=ITEM_COUNT) return 0;
  const char* id=ITEMS[item].id;
  if(!strcmp(id,"OBJ_0037")){ hitBonus=0; return 0; }               // Dague
  if(!strcmp(id,"OBJ_0001")){ hitBonus=0; return 2; }               // Epee usee
  if(!strcmp(id,"OBJ_0002")){ hitBonus=0; return 3; }               // Hache de bucheron
  if(!strcmp(id,"OBJ_0008")){ hitBonus=1; isBow=true; return 2; }   // Arc court
  if(!strcmp(id,"OBJ_0020")){ hitBonus=2; isBow=true; return 3; }   // Arc de chasseur
  if(!strcmp(id,"OBJ_0032")){ hitBonus=1; return 4; }               // Epee de fer
  return 0;
}
bool is_weapon_item(int item){
  if(item<0||item>=ITEM_COUNT) return false;
  return ITEMS[item].cat==IC_WEAPON;
}
int armor_defense(const Character& c){
  int def=0;
  int sh=item_index("OBJ_0023"), hl=item_index("OBJ_0010"), cp=item_index("OBJ_0024");
  if(sh>=0 && c.inv.has(sh,1)) def+=2;   // Bouclier de voyage
  if(hl>=0 && c.inv.has(hl,1)) def+=1;   // Casque de mineur
  if(cp>=0 && c.inv.has(cp,1)) def+=1;   // Cape de pluie
  return def;
}
bool is_consumable_field(int item){
  if(item<0||item>=ITEM_COUNT) return false;
  const char* id=ITEMS[item].id;
  return !strcmp(id,"OBJ_0005")||!strcmp(id,"OBJ_0033")||!strcmp(id,"OBJ_0026")||
         !strcmp(id,"OBJ_0034")||!strcmp(id,"OBJ_0003")||!strcmp(id,"OBJ_0011");
}
bool use_item_field(Character& c, int item, char* out, int cap){
  if(item<0||item>=ITEM_COUNT||!c.inv.has(item,1)) return false;
  const char* id=ITEMS[item].id;
  auto heal=[&](int n){ c.BL-=n; if(c.BL<0) c.BL=0; };
  if(!strcmp(id,"OBJ_0005")){ if(c.BL<=0){ snprintf(out,cap,"%s",trc("Santé déjà au maximum.")); return false; } c.inv.remove(item,1); heal(8); snprintf(out,cap,"%s",trc("Petite potion : -8 blessures.")); return true; }
  if(!strcmp(id,"OBJ_0033")){ if(c.BL<=0){ snprintf(out,cap,"%s",trc("Santé déjà au maximum.")); return false; } c.inv.remove(item,1); heal(16); snprintf(out,cap,"%s",trc("Grande potion : -16 blessures.")); return true; }
  if(!strcmp(id,"OBJ_0026")){ if(c.BL<=0 && c.mana>=c.manaMax){ snprintf(out,cap,"%s",trc("Tu es déjà en pleine forme.")); return false; } c.inv.remove(item,1); heal(6); c.mana+=2; if(c.mana>c.manaMax)c.mana=c.manaMax; snprintf(out,cap,"%s",trc("Miel sauvage : +6 PV, +2 PO.")); return true; }
  if(!strcmp(id,"OBJ_0034")){ if(c.BL<=0){ snprintf(out,cap,"%s",trc("Santé déjà au maximum.")); return false; } c.inv.remove(item,1); heal(3); snprintf(out,cap,"%s",trc("Herbe mâchée : -3 blessures.")); return true; }
  if(!strcmp(id,"OBJ_0003")){ c.inv.remove(item,1); c.torchFuel=100; snprintf(out,cap,"%s",trc("Torche allumée : lumière 100%.")); return true; }
  if(!strcmp(id,"OBJ_0011")){ c.inv.remove(item,1); c.torchFuel+=80; if(c.torchFuel>100)c.torchFuel=100; snprintf(out,cap,trc("Lanterne remplie : lumière %d%%."),c.torchFuel); return true; }
  return false;
}
const char* item_desc(int item){
  if(item<0||item>=ITEM_COUNT) return "";
  const char* id=ITEMS[item].id;
  if(!strcmp(id,"OBJ_0001")) return "Arme : 1D + 2 dégâts";
  if(!strcmp(id,"OBJ_0002")) return "Arme : 1D + 3 dégâts";
  if(!strcmp(id,"OBJ_0003")) return "Recharge la lumière dans la Mine (100%)";
  if(!strcmp(id,"OBJ_0004")) return "Fuite garantie ou sortie rapide de Mine";
  if(!strcmp(id,"OBJ_0005")) return "Soigne 8 blessures (combat / sac)";
  if(!strcmp(id,"OBJ_0008")) return "Arc : 1D + 2 dégâts, +1 toucher";
  if(!strcmp(id,"OBJ_0009")) return "Double les minerais extraits des filons";
  if(!strcmp(id,"OBJ_0010")) return "Armure : -1 dégât subi + quête Doran";
  if(!strcmp(id,"OBJ_0011")) return "Recharge la lumière (+80%) + forge";
  if(!strcmp(id,"OBJ_0012")) return "Minerai de forge (avec lingot rouillé)";
  if(!strcmp(id,"OBJ_0013")) return "Forge : fondre avec du charbon";
  if(!strcmp(id,"OBJ_0019")) return "Lumière permanente inépuisable en Mine";
  if(!strcmp(id,"OBJ_0020")) return "Arc : 1D + 3 dégâts, +2 toucher";
  if(!strcmp(id,"OBJ_0021")) return "Double le rendement minier";
  if(!strcmp(id,"OBJ_0023")) return "Armure passive : -2 dégâts subis";
  if(!strcmp(id,"OBJ_0024")) return "Armure : -1 dégât + moins d'embuscades";
  if(!strcmp(id,"OBJ_0025")) return "Corde réutilisable : sortie/fuite sûre";
  if(!strcmp(id,"OBJ_0026")) return "+6 PV et +2 PO (ou alchimie)";
  if(!strcmp(id,"OBJ_0027")) return "+2 dégâts avec un arc (munitions)";
  if(!strcmp(id,"OBJ_0031")) return "Lingot prêt pour forger armes & bouclier";
  if(!strcmp(id,"OBJ_0032")) return "Arme : 1D + 4 dégâts, +1 toucher";
  if(!strcmp(id,"OBJ_0033")) return "Soigne 16 blessures (combat / sac)";
  if(!strcmp(id,"OBJ_0034")) return "Alchimie / quêtes / -3 blessures";
  if(!strcmp(id,"OBJ_0035")) return "Alchimie (potions) & quête d'Agnès";
  if(!strcmp(id,"OBJ_0036")) return "Artisanat : flèches & bouclier";
  if(!strcmp(id,"OBJ_0037")) return "Arme légère : 1D dégâts";
  if(!strcmp(id,"OBJ_0040")) return "+2 CO et +1 dégât en combat";
  if(ITEMS[item].cat==IC_QUEST||ITEMS[item].cat==IC_KEY) return "Objet clé / quête";
  if(ITEMS[item].cat==IC_MAP) return "Carte d'exploration";
  return "Équipement d'aventurier";
}
int item_base_price(int item){
  if(item<0||item>=ITEM_COUNT) return 0;
  const char* id=ITEMS[item].id;
  if(!strcmp(id,"OBJ_0005")) return 12;
  if(!strcmp(id,"OBJ_0033")) return 28;
  if(!strcmp(id,"OBJ_0003")) return 5;
  if(!strcmp(id,"OBJ_0004")) return 5;
  if(!strcmp(id,"OBJ_0025")) return 16;
  if(!strcmp(id,"OBJ_0011")) return 6;
  if(!strcmp(id,"OBJ_0012")) return 7;
  if(!strcmp(id,"OBJ_0013")) return 8;
  if(!strcmp(id,"OBJ_0036")) return 4;
  if(!strcmp(id,"OBJ_0034")) return 5;
  if(!strcmp(id,"OBJ_0035")) return 5;
  if(!strcmp(id,"OBJ_0026")) return 10;
  if(!strcmp(id,"OBJ_0027")) return 5;
  if(!strcmp(id,"OBJ_0031")) return 18;
  if(!strcmp(id,"OBJ_0037")) return 8;
  if(!strcmp(id,"OBJ_0001")) return 14;
  if(!strcmp(id,"OBJ_0002")) return 20;
  if(!strcmp(id,"OBJ_0008")) return 30;
  if(!strcmp(id,"OBJ_0020")) return 55;
  if(!strcmp(id,"OBJ_0032")) return 48;
  if(!strcmp(id,"OBJ_0023")) return 36;
  if(!strcmp(id,"OBJ_0024")) return 24;
  if(!strcmp(id,"OBJ_0019")) return 40;
  if(!strcmp(id,"OBJ_0009")) return 14;
  return 10;
}
int apply_au_discount(int price, int au){
  int disc = (au > 10) ? (au - 10) * 2 : 0;   // jusqu'a -20% avec Aura
  if(disc > 20) disc = 20;
  int out = price * (100 - disc) / 100;
  return out < 1 ? 1 : out;
}
int item_sell_price(int item){ int p=item_base_price(item)/2; return p<1?1:p; }
bool is_item_sellable(int item){
  if(item<0||item>=ITEM_COUNT) return false;
  uint8_t c=ITEMS[item].cat;
  if(c==IC_QUEST || c==IC_KEY || c==IC_MAP) return false;
  if(!strcmp(ITEMS[item].id,"OBJ_0040") || !strcmp(ITEMS[item].id,"OBJ_0010")) return false;
  return item!=player().weapon;
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
  static char buf[2][256]; static int k=0; char* b=buf[k^=1];
  strncpy(b,s,255); b[255]=0;
  if(rename) repl_all(b,256,"Caithness",c.name);
  if(!c.female){
    // ordre : formes longues (avec article) d'abord
    static const char* const M[][2]={
      {"la voyageuse","le voyageur"},{"voyageuse","voyageur"},   // FR
      {"la viajera","el viajero"},{"viajera","viajero"},{"Bienvenida","Bienvenido"},   // ES
      {"die Reisende","der Reisende"},{", Reisende",", Reisender"},   // DE
      {"la viaggiatrice","il viaggiatore"},{"viaggiatrice","viaggiatore"},{"Benvenuta","Benvenuto"},   // IT
      {"fille des fées","fils des fées"},{"fae daughter","fae son"},{"hija de las hadas","hijo de las hadas"},
      {"Feentochter","Feensohn"},{"figlia delle fate","figlio delle fate"} };   // "enfant des fees" (Orwenn, Voyageur)
    for(const auto& r:M) repl_all(b,256,r[0],r[1]);
  } else {
    static const char* const F[][2]={
      {"Un voyageur","Une voyageuse"},{"un voyageur","une voyageuse"},{"le voyageur","la voyageuse"},
      {"El viajero","La viajera"},{"el viajero","la viajera"},
      {"Ein Reisender","Eine Reisende"},{"der Reisende","die Reisende"},
      {"Un viaggiatore","Una viaggiatrice"},{"il viaggiatore","la viaggiatrice"} };
    for(const auto& r:F) repl_all(b,256,r[0],r[1]);
  }
  return b;
}
}
