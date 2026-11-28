// ASTERIA (Gamebuino AKA) - Echoppe : Acheter / Vendre, remise Aura (AU), stock & description.
#include "scene/shop_scene.h"
#include "i18n.h"
#include "platform/gb_port.h"
#include "ui/text.h"
#include "ui/item_ui.h"
#include "player.h"
#include "audio/jingle.h"
#include "content_i18n.h"
#include "generated/asteria_gen.h"
#include <cstdio>

namespace asteria {
using namespace data;

struct ShopOffer { const char* id; int qty; int basePrice; };
static const ShopOffer OFFERS[] = {
  {"OBJ_0005", 1, 12},  // Petite potion de soin
  {"OBJ_0033", 1, 28},  // Grande potion de soin
  {"OBJ_0003", 1,  5},  // Torche
  {"OBJ_0011", 1,  6},  // Huile pour lanterne
  {"OBJ_0004", 1,  5},  // Corde
  {"OBJ_0025", 1, 16},  // Corde solide
  {"OBJ_0012", 1,  7},  // Charbon
  {"OBJ_0036", 1,  4},  // Bois
  {"OBJ_0034", 1,  6},  // Herbe medicinale : la cueillette est limitee (19 points en foret) ;
  {"OBJ_0035", 1,  6},  // Champignon       : si le joueur en vend ou en distille, Agnes (Q4) et Iris (Q8) restent faisables
  {"OBJ_0027", 3, 12},  // 3x Fleches renforcees
  {"OBJ_0008", 1, 30},  // Arc court
  {"OBJ_0024", 1, 24},  // Cape de pluie
  {"OBJ_0042", 1, 30},  // Veste de cuir      (v1.5 : emplacements armure / pantalon / bottes)
  {"OBJ_0044", 1, 22},  // Braies renforcees
  {"OBJ_0045", 1, 20},  // Bottes de cuir
};
static const int NOFFER = (int)(sizeof(OFFERS)/sizeof(OFFERS[0]));

static int tab = 0;   // 0=Acheter, 1=Vendre
static int sel = 0;
static char msg[64] = {0};

static int build_sell_list(int* outIdx){
  int n=0;
  const Inventory& inv = player().inv;
  for(int i=0; i<inv.n; ++i){
    if(is_item_sellable(inv.slots[i].item)) outIdx[n++] = i;
  }
  return n;
}

ShopScene& shop_scene(){ static ShopScene s; return s; }
void ShopScene::enter(){ tab=0; sel=0; msg[0]=0; }

void ShopScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(p&(gb::BTN_LEFT|gb::BTN_RIGHT)){ tab^=1; sel=0; msg[0]=0; audio::sfx_select(); }

  int sellIdx[Inventory::CAP];
  int cnt = (tab==0) ? NOFFER : build_sell_list(sellIdx);
  if(cnt > 0 && sel >= cnt) sel = cnt - 1;

  if(cnt > 0){
    if(p&gb::BTN_DOWN){ sel=(sel+1)%cnt; msg[0]=0; audio::sfx_select(); }
    if(p&gb::BTN_UP){   sel=(sel+cnt-1)%cnt; msg[0]=0; audio::sfx_select(); }
  }

  if(p&gb::BTN_A && cnt > 0){
    if(tab==0){
      const ShopOffer& of = OFFERS[sel];
      int it = item_index(of.id);
      int pr = apply_au_discount(of.basePrice, player().AU + eq_stats(player()).au);
      if(it < 0){ snprintf(msg,64,"%s",trc("Objet indisponible.")); audio::sfx_error(); }
      else if(player().gold < pr){ snprintf(msg,64,"%s",i18n::tr(i18n::SH_NOGOLD)); audio::sfx_error(); }
      else if(!player().inv.add(it, of.qty)){ snprintf(msg,64,"%s",trc("Inventaire plein !")); audio::sfx_error(); }
      else {
        player().gold -= pr;
        audio::sfx_coin();
        if(of.qty>1) snprintf(msg,64,trc("Acheté : %dx %s"),of.qty,trc(ITEMS[it].name));
        else snprintf(msg,64,"%s %s",i18n::tr(i18n::SH_BOUGHT),trc(ITEMS[it].name));
      }
    } else {
      int slotIdx = sellIdx[sel];
      int it = player().inv.slots[slotIdx].item;
      int val = item_sell_price(it);
      if(player().inv.remove(it, 1)){
        player().gold += val;
        audio::sfx_coin();
        snprintf(msg,64,trc("Vendu : %s (+%d or)"),trc(ITEMS[it].name),val);
      }
    }
  }
  if(p&gb::BTN_B) m.set(SceneId::WORLD);
}

void ShopScene::render(){
  gb::clear(gb::rgb(30,24,16));
  ui::text_big(18,8,i18n::tr(i18n::SH_TITLE),gb::rgb(235,215,140));
  char gbuf[32]; snprintf(gbuf,32,"%s %d",i18n::tr(i18n::SH_GOLD),player().gold);
  ui::gold(306-ui::text_w(gbuf),12,gbuf,true);

  // Onglets Acheter / Vendre
  gb::fill_rect(18,34,108,16, tab==0 ? gb::rgb(78,58,26) : gb::rgb(42,34,24));
  gb::fill_rect(132,34,108,16, tab==1 ? gb::rgb(78,58,26) : gb::rgb(42,34,24));
  ui::text(42,38,trc("Acheter"), tab==0 ? gb::rgb(248,224,135) : gb::rgb(175,165,140));
  ui::text(158,38,trc("Vendre"), tab==1 ? gb::rgb(248,224,135) : gb::rgb(175,165,140));
  int au = player().AU + eq_stats(player()).au;   // collier d'hermine compris
  int disc = (au > 10) ? ((au - 10) * 2) : 0; if(disc>20) disc=20;
  if(disc>0 && tab==0){ char db[20]; snprintf(db,sizeof(db),"AU -%d%%",disc); ui::text(250,38,db,gb::rgb(145,215,150)); }

  int sellIdx[Inventory::CAP];
  int cnt = (tab==0) ? NOFFER : build_sell_list(sellIdx);
  const int VIS=5;   // rangees de 25 px : icone 24x24 + nom + prix
  int top = (sel >= VIS) ? (sel - VIS + 1) : 0;
  if(top + VIS > cnt) top = cnt - VIS;
  if(top < 0) top = 0;

  int activeItem = -1;
  if(cnt == 0){
    ui::text(24,78,trc("Aucun objet vendable dans le sac."),gb::rgb(175,165,140));
  } else {
    for(int r=0; r<VIS && (top+r)<cnt; ++r){
      int i=top+r; int y=60+r*25;
      if(i==sel){ gb::fill_rect(14,y-6,292,25,gb::rgb(62,46,20)); ui::gold(18,y,">",true); }
      int it=-1, qty=0, price=0;
      char nm[48];
      if(tab==0){
        const ShopOffer& of = OFFERS[i];
        it = item_index(of.id);
        price = apply_au_discount(of.basePrice, player().AU + eq_stats(player()).au);
        int ownedIdx = (it>=0) ? player().inv.find(it) : -1;
        qty = (ownedIdx>=0) ? (int)player().inv.slots[ownedIdx].qty : 0;
        if(it>=0 && of.qty>1) snprintf(nm,sizeof(nm),trc("%dx %s (sac:%d)"),of.qty,trc(ITEMS[it].name),qty);
        else if(it>=0) snprintf(nm,sizeof(nm),trc("%s (sac:%d)"),trc(ITEMS[it].name),qty);
        else snprintf(nm,sizeof(nm),"%s",of.id);
      } else {
        const ItemStack& st = player().inv.slots[sellIdx[i]];
        it = st.item; qty = st.qty; price = item_sell_price(it);
        snprintf(nm,sizeof(nm),"%s x%d",trc(ITEMS[it].name),qty);
      }
      if(i==sel) activeItem = it;
      if(it>=0) item_ui::draw_icon(it,30,y-6,false);
      ui::text(60,y,nm,gb::rgb(235,225,190));
      char pr[16]; snprintf(pr,sizeof(pr),trc("%d or"),price);
      ui::text(302-ui::text_w(pr),y,pr,gb::rgb(230,205,120));
    }
  }

  gb::fill_rect(14,182,292,24,gb::rgb(20,16,12));
  if(msg[0]) ui::text(20,188,msg,gb::rgb(190,235,175));
  else if(activeItem>=0) ui::text(20,188,trc(item_desc(activeItem)),gb::rgb(200,190,160));

  ui::text(18,216,trc("</> Onglet   A: valider   B: sortir"),gb::rgb(175,165,135));
}
}
