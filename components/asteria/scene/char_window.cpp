// ASTERIA (Gamebuino AKA) - Fenetre personnage (voir char_window.h).
#include "scene/char_window.h"
#include "scene/combat_scene.h"   // spell_known / spell_cost / spell_short / spell_fx
#include "ui/text.h"
#include "ui/item_ui.h"
#include "platform/gb_port.h"
#include "generated/asteria_gen.h"
#include "generated/asteria_hero.h"
#include "generated/asteria_spells_gen.h"
#include "audio/jingle.h"
#include "content_i18n.h"
#include "player.h"
#include <cstdio>
namespace asteria { namespace charwin {
using namespace data;

static bool s_open = false;
static int  s_tab = 0;          // 0 = Sac, 1 = Sorts
static int  s_sel = 0;          // case du sac
static int  s_ssel = 0;         // ligne de la liste des sorts
static const char* s_msg = nullptr;
static char s_buf[72];

// Grille du sac : 6 colonnes, 3 rangees visibles (defilement par rangee)
static const int COLS = 6, ROWS = 3, CELL = 28, GX = 126, GY = 88;
// Emplacements d'equipement affiches au-dessus du sac (l'arme est celle equipee ;
// les autres sont passifs : il suffit de les porter dans le sac)
static const char* const EQUIP_IDS[4] = { "OBJ_0023", "OBJ_0010", "OBJ_0024", "OBJ_0040" };

static int field_spells(int* out){            // sorts connus, dans l'ordre du grimoire
  int n = 0;
  for(int i = 0; i < SPELL_COUNT; ++i) if(spell_known(i)) out[n++] = i;
  return n;
}
static bool castable_here(int s){ return s == 4 || s == 7; }   // Soins, Guerison majeure

static uint32_t s_rng = 0;
static int d6(){ if(!s_rng) s_rng = gb::millis() | 1u; s_rng ^= s_rng << 13; s_rng ^= s_rng >> 17; s_rng ^= s_rng << 5; return (int)(s_rng % 6) + 1; }

void open(int tab){ s_open = true; s_tab = tab; s_msg = nullptr; if(s_sel >= player().inv.n) s_sel = 0; }
bool is_open(){ return s_open; }
void close(){ s_open = false; s_msg = nullptr; }

static void bag_update(uint32_t p){
  Character& c = player();
  int n = c.inv.n; if(n == 0) return;
  if(s_sel >= n) s_sel = n - 1;
  int old = s_sel;
  if(p & gb::BTN_RIGHT) s_sel = (s_sel + 1) % n;
  if(p & gb::BTN_LEFT)  s_sel = (s_sel + n - 1) % n;
  if(p & gb::BTN_DOWN){ s_sel += COLS; if(s_sel >= n) s_sel = (s_sel % COLS < n) ? s_sel % COLS : 0; }
  if(p & gb::BTN_UP){   s_sel -= COLS; if(s_sel < 0){ int last = ((n - 1) / COLS) * COLS + (s_sel + COLS); s_sel = last < n ? last : n - 1; } }
  if(s_sel != old) s_msg = nullptr;
  if(!(p & gb::BTN_A)) return;
  int it = c.inv.slots[s_sel].item;
  if(is_weapon_item(it)){
    c.weapon = it;
    snprintf(s_buf, sizeof(s_buf), trc("Équipé : %s"), trc(ITEMS[it].name));
    s_msg = s_buf;
  } else if(is_consumable_field(it)){
    if(use_item_field(c, it, s_buf, sizeof(s_buf))) audio::sfx_coin(); else audio::sfx_error();
    s_msg = s_buf;
    if(s_sel >= c.inv.n && c.inv.n > 0) s_sel = c.inv.n - 1;   // pile epuisee
  }
}

static void spell_update(uint32_t p){
  int L[SPELL_COUNT]; int n = field_spells(L); if(n == 0) return;
  if(s_ssel >= n) s_ssel = n - 1;
  if(p & gb::BTN_DOWN){ s_ssel = (s_ssel + 1) % n; s_msg = nullptr; }
  if(p & gb::BTN_UP){   s_ssel = (s_ssel + n - 1) % n; s_msg = nullptr; }
  if(!(p & gb::BTN_A)) return;
  int s = L[s_ssel]; Character& c = player();
  if(!castable_here(s)){ s_msg = "Ce sort ne sert qu'en combat."; audio::sfx_error(); return; }
  int cost = spell_cost(s);
  if(c.BL <= 0){ s_msg = "Tu n'as aucune blessure."; audio::sfx_error(); return; }
  if(c.mana < cost){ s_msg = "Pas assez de pouvoir !"; audio::sfx_error(); return; }
  c.mana -= cost; int h = 0; for(int i = 0; i < (s == 4 ? 2 : 4); ++i) h += d6();
  c.BL -= h; if(c.BL < 0) c.BL = 0;
  snprintf(s_buf, sizeof(s_buf), trc("%s : -%d blessures."), trc(spell_short(s)), h);
  s_msg = s_buf; audio::sfx_spell();
}

void update(uint32_t p){
  if(p & gb::BTN_B){ close(); return; }
  if(p & gb::BTN_RUN){ s_tab ^= 1; s_msg = nullptr; return; }
  if(s_tab == 0) bag_update(p); else spell_update(p);
}

// --- Rendu -------------------------------------------------------------------
static void frame(int x, int y, int w, int h, gb::Color c){
  gb::fill_rect(x, y, w, 2, c); gb::fill_rect(x, y + h - 2, w, 2, c);
  gb::fill_rect(x, y, 2, h, c); gb::fill_rect(x + w - 2, y, 2, h, c);
}
static void bar(int x, int y, int w, int v, int vmax, gb::Color c){
  gb::fill_rect(x, y, w, 5, gb::rgb(50,40,28));
  if(vmax > 0 && v > 0){ int f = w * (v > vmax ? vmax : v) / vmax; gb::fill_rect(x, y, f, 5, c); }
}
static void hero_x2(int x, int y){                  // portrait : sprite de face, double taille
  const spr::Sprite& s = spr::HERO[0][1];
  for(int j = 0; j < s.h; ++j) for(int i = 0; i < s.w; ++i){
    uint16_t c = s.px[j * s.w + i]; if(c != spr::KEY) gb::fill_rect(x + i*2, y + j*2, 2, 2, c);
  }
}
static void slot_box(int x, int y, int item, bool owned){
  gb::fill_rect(x, y, 26, 26, gb::rgb(34,27,16)); frame(x, y, 26, 26, gb::rgb(70,56,32));
  if(item >= 0 && owned) item_ui::draw_icon(item, x + 1, y + 1, false);
}

void render(){
  if(!s_open) return;
  const Character& c = player();
  const gb::Color GOLDC = gb::rgb(200,170,90), TXT = gb::rgb(235,225,190), DIM = gb::rgb(170,160,132);
  gb::fill_rect(6,30,308,206,gb::rgb(20,16,10));
  gb::fill_rect(6,30,308,2,GOLDC); gb::fill_rect(6,234,308,2,GOLDC);
  gb::fill_rect(118,34,1,136,gb::rgb(70,56,32));        // separation des colonnes

  // Colonne gauche : identite, jauges, caracteristiques
  hero_x2(12,36);
  ui::gold(42,36,c.name,true);
  char ln[48];
  snprintf(ln,sizeof(ln),trc("Niv. %d"),c.level);   ui::text(42,53,ln,TXT);
  snprintf(ln,sizeof(ln),trc("%d or"),c.gold);       ui::text(42,68,ln,gb::rgb(245,215,95));
  int pv = c.VIT - c.BL; if(pv < 0) pv = 0;
  snprintf(ln,sizeof(ln),"PV %d/%d",pv,c.VIT);       ui::text(12,84,ln,TXT);  bar(12,99,100,pv,c.VIT,gb::rgb(200,60,50));
  snprintf(ln,sizeof(ln),"PM %d/%d",c.mana,c.manaMax); ui::text(12,106,ln,TXT); bar(12,121,100,c.mana,c.manaMax,gb::rgb(80,120,230));
  snprintf(ln,sizeof(ln),"IN %d  CO %d",c.IN,c.CO);   ui::text(12,128,ln,TXT);
  snprintf(ln,sizeof(ln),"MA %d  AU %d",c.MA,c.AU);   ui::text(12,142,ln,TXT);
  snprintf(ln,sizeof(ln),trc("Armure %d"),armor_defense(c)); ui::text(12,156,ln,TXT);

  // Onglets
  const char* T0 = trc("Sac"); const char* T1 = trc("Sorts");
  int w0 = ui::text_w(T0) + 12, w1 = ui::text_w(T1) + 12;
  gb::fill_rect(124,34,w0,16,s_tab == 0 ? gb::rgb(60,45,20) : gb::rgb(30,24,14));
  gb::fill_rect(128 + w0,34,w1,16,s_tab == 1 ? gb::rgb(60,45,20) : gb::rgb(30,24,14));
  if(s_tab == 0) ui::gold(130,35,T0,true); else ui::text(130,35,T0,DIM);
  if(s_tab == 1) ui::gold(134 + w0,35,T1,true); else ui::text(134 + w0,35,T1,DIM);

  const char* name = nullptr; const char* desc = nullptr; char nameBuf[64];
  if(s_tab == 0){
    // Equipement : arme equipee + objets passifs portes
    slot_box(126,56,c.weapon,c.weapon >= 0);
    for(int k = 0; k < 4; ++k){ int it = item_index(EQUIP_IDS[k]); slot_box(160 + k*30,56,it,it >= 0 && c.inv.has(it,1)); }
    gb::fill_rect(126,84,182,1,gb::rgb(70,56,32));
    // Sac
    int n = c.inv.n;
    if(n == 0){ ui::text(GX,GY + 8,trc("Le sac est vide."),DIM); }
    else {
      if(s_sel >= n) s_sel = n - 1;
      int row = s_sel / COLS, top = row >= ROWS ? row - ROWS + 1 : 0;
      for(int r = 0; r < ROWS; ++r) for(int col = 0; col < COLS; ++col){
        int idx = (top + r) * COLS + col; int x = GX + col*CELL, y = GY + r*CELL;
        gb::fill_rect(x,y,CELL - 2,CELL - 2,gb::rgb(34,27,16));
        if(idx >= n) continue;
        const ItemStack& st = c.inv.slots[idx];
        item_ui::draw_icon(st.item,x + 1,y + 1,false);
        if(st.qty > 1){ char q[12]; snprintf(q,sizeof(q),"%d",(int)st.qty); int tw = ui::text_w(q);
          gb::fill_rect(x + CELL - 3 - tw,y + CELL - 13,tw + 1,11,gb::rgb(20,16,10)); ui::text(x + CELL - 3 - tw,y + CELL - 16,q,TXT); }
        if(st.item == c.weapon) frame(x,y,CELL - 2,CELL - 2,gb::rgb(245,215,95));
        if(idx == s_sel) frame(x - 1,y - 1,CELL,CELL,gb::rgb(255,255,255));
      }
      if(top > 0) ui::text(300,GY - 4,"^",DIM);
      if((top + ROWS) * COLS < n) ui::text(300,GY + ROWS*CELL - 12,"v",DIM);
      const ItemStack& st = c.inv.slots[s_sel];
      snprintf(nameBuf,sizeof(nameBuf),st.qty > 1 ? "%s x%d" : "%s",trc(ITEMS[st.item].name),(int)st.qty);
      name = nameBuf; desc = trc(item_desc(st.item));
    }
  } else {
    int L[SPELL_COUNT]; int n = field_spells(L);
    if(n == 0) ui::text(130,60,trc("Aucun sort connu."),DIM);
    else {
      if(s_ssel >= n) s_ssel = n - 1;
      const int VIS = 7; int top = s_ssel >= VIS ? s_ssel - VIS + 1 : 0;
      for(int r = 0; r < VIS && top + r < n; ++r){
        int s = L[top + r], y = 54 + r*16; bool here = castable_here(s);
        if(top + r == s_ssel) gb::fill_rect(124,y - 1,186,16,gb::rgb(60,45,20));
        ui::text(130,y,trc(spell_short(s)),here ? TXT : DIM);
        char cs[12]; snprintf(cs,sizeof(cs),"%d",spell_cost(s));
        ui::text(304 - ui::text_w(cs),y,cs,c.mana >= spell_cost(s) ? gb::rgb(150,170,240) : gb::rgb(110,100,90));
      }
      int s = L[s_ssel];
      snprintf(nameBuf,sizeof(nameBuf),trc("%s (%d PM)"),trc(spell_short(s)),spell_cost(s));
      name = nameBuf; desc = trc(spell_fx(s));   // en gris dans la liste : sort de combat seulement
    }
  }

  // Bas : nom, description (ou resultat de la derniere action), aide
  gb::fill_rect(10,172,300,1,gb::rgb(70,56,32));
  if(name) ui::gold(12,175,name,true);
  if(s_msg) ui::text(12,192,trc(s_msg),gb::rgb(245,215,95));
  else if(desc){ char W[2][64]; int k = item_ui::wrap_text(desc,296,W,2);
    for(int i = 0; i < k; ++i) ui::text(12,192 + i*14,W[i],gb::rgb(185,225,175)); }
  ui::text(12,219,trc(s_tab == 0 ? "A: utiliser  RUN: sorts  B: fermer" : "A: lancer  RUN: sac  B: fermer"),DIM);
}

}}
