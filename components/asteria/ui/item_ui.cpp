// ASTERIA (Gamebuino AKA) - Icones d'objets et fenetre "Objet obtenu" (voir item_ui.h).
#include "ui/item_ui.h"
#include "ui/text.h"
#include "platform/gb_port.h"
#include "generated/asteria_gen.h"
#include "generated/asteria_icons.h"
#include "content_i18n.h"
#include "player.h"
#include <cstdio>
#include <cstring>
namespace asteria { namespace item_ui {

void draw_icon(int item, int x, int y, bool big){
  int slot = (item == GOLD) ? icons::ICON_GOLD : item;
  if(slot < 0 || slot >= icons::ICON_COUNT) return;
  if(big) gb::draw_image_key(icons::ICON48[slot], 48, 48, x, y, icons::KEY);
  else    gb::draw_image_key(icons::ICON24[slot], 24, 24, x, y, icons::KEY);
}

int wrap_text(const char* s, int maxw, char out[][64], int nmax){
  int n = 0;
  while(s && *s && n < nmax){
    while(*s == ' ') ++s;
    if(!*s) break;
    // plus long prefixe (coupe a un espace) qui tient dans maxw
    char buf[64]; int len = 0, cut = -1;
    while(s[len] && len < 63){
      buf[len] = s[len]; buf[len + 1] = 0;
      if(ui::text_w(buf) > maxw) break;
      if(s[len] == ' ') cut = len;
      ++len;
    }
    if(s[len] && len < 63 && s[len] != ' ' && cut > 0) len = cut;   // ne pas couper un mot
    memcpy(out[n], s, len); out[n][len] = 0;
    ++n; s += len;
  }
  return n;
}

// --- File des gains --------------------------------------------------------
struct Gain { int16_t item; int16_t qty; };
static Gain s_q[8];
static int  s_n = 0;

void pickup_push(int item, int qty){
  if(qty <= 0) return;
  for(int i = 0; i < s_n; ++i)                          // meme objet deja en attente : on cumule
    if(s_q[i].item == item){ s_q[i].qty = (int16_t)(s_q[i].qty + qty); return; }
  if(s_n < 8) s_q[s_n++] = { (int16_t)item, (int16_t)qty };
}
bool pickup_active(){ return s_n > 0; }
void pickup_clear(){ s_n = 0; }
void pickup_update(uint32_t p){
  if(s_n > 0 && (p & (gb::BTN_A | gb::BTN_B))){
    for(int i = 1; i < s_n; ++i) s_q[i - 1] = s_q[i];
    --s_n;
  }
}

void pickup_render(){
  if(!s_n) return;
  const Gain& g = s_q[0];
  const gb::Color BG = gb::rgb(20,16,10), GOLDC = gb::rgb(200,170,90);
  gb::fill_rect(8,150,304,84,BG);
  gb::fill_rect(8,150,304,2,GOLDC); gb::fill_rect(8,232,304,2,GOLDC);
  // cadre de l'icone
  gb::fill_rect(15,165,54,54,gb::rgb(58,44,22)); gb::fill_rect(17,167,50,50,gb::rgb(34,27,16));
  draw_icon(g.item, 18, 168, true);

  ui::gold(78,154,trc("Objet obtenu"),true);
  char name[64]; const char* desc;
  if(g.item == GOLD){
    snprintf(name,sizeof(name),trc("%d pièces d'or"),(int)g.qty);
    desc = trc("De quoi remplir ta bourse.");
  } else {
    const char* nm = trc(data::ITEMS[g.item].name);
    if(g.qty > 1) snprintf(name,sizeof(name),"%s x%d",nm,(int)g.qty); else snprintf(name,sizeof(name),"%s",nm);
    desc = trc(item_desc(g.item));
  }
  ui::text(78,174,name,gb::rgb(245,225,150));
  char L[2][64]; int n = wrap_text(desc,226,L,2);
  for(int i = 0; i < n; ++i) ui::text(78,192 + i*15,L[i],gb::rgb(185,225,175));
  if(s_n > 1){ char more[16]; snprintf(more,sizeof(more),"+%d",s_n - 1); ui::text(300 - ui::text_w(more),154,more,gb::rgb(170,160,132)); }
  ui::text(300 - ui::text_w("A"),216,"A",gb::rgb(170,160,132));
}

}}
