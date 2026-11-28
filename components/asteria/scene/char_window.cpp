// ASTERIA (Gamebuino AKA) - Fenetre personnage (voir char_window.h).
#include "scene/char_window.h"
#include "scene/combat_scene.h"   // spell_known / spell_cost / spell_short / spell_fx
#include "ui/text.h"
#include "ui/item_ui.h"
#include "platform/gb_port.h"
#include "generated/asteria_gen.h"
#include "ui/hero_sprite.h"
#include "generated/asteria_spells_gen.h"
#include "audio/jingle.h"
#include "content_i18n.h"
#include "player.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace asteria { namespace charwin {
using namespace data;

enum { TAB_EQUIP, TAB_BAG, TAB_SPELLS, TAB_N };
static bool s_open = false;
static int  s_tab = TAB_EQUIP;
static int  s_slot = EQ_BODY;   // emplacement selectionne sur la poupee
static bool s_pick = false;     // liste de choix d'un objet pour s_slot ouverte
static int  s_psel = 0;         // ligne de cette liste
static int  s_sel = 0;          // case du sac
static int  s_ssel = 0;         // ligne de la liste des sorts
static const char* s_msg = nullptr;
static char s_buf[72];

// Grille du sac : 6 colonnes, 4 rangees visibles (defilement par rangee)
static const int COLS = 6, ROWS = 4, CELL = 28, GX = 126, GY = 54;

// Poupee : position (coin haut-gauche, cases 26x26) de chaque emplacement, dans l'ordre
// d'EqSlot. La main droite du personnage est a gauche de l'ecran (vue de face).
static const int16_t SLOT_XY[EQ_COUNT][2] = {
  {152, 96},  {254, 96},              // main droite, main gauche
  {124,126},  {152,126},              // anneaux de la main droite
  {254,126},  {282,126},              // anneaux de la main gauche
  {234, 60},                          // collier
  {203, 82},  {203,112},  {203,142},  // armure, pantalon, bottes
  {203, 52} };                        // casque
static int sx(int s){ return SLOT_XY[s][0]; }
static int sy(int s){ return SLOT_XY[s][1]; }

static int field_spells(int* out){            // sorts connus, dans l'ordre du grimoire
  int n = 0;
  for(int i = 0; i < SPELL_COUNT; ++i) if(spell_known(i)) out[n++] = i;
  return n;
}
static bool castable_here(int s){ return s == 4 || s == 7; }   // Soins, Guerison majeure

static uint32_t s_rng = 0;
static int d6(){ if(!s_rng) s_rng = gb::millis() | 1u; s_rng ^= s_rng << 13; s_rng ^= s_rng >> 17; s_rng ^= s_rng << 5; return (int)(s_rng % 6) + 1; }

void open(int tab){ s_open = true; s_tab = tab; s_pick = false; s_msg = nullptr; if(s_sel >= player().inv.n) s_sel = 0; }
bool is_open(){ return s_open; }
void close(){ s_open = false; s_pick = false; s_msg = nullptr; }

// Objets du sac compatibles avec un emplacement (+ "Retirer" en tete si occupe et retirable)
static int pick_list(int slot, int* out){
  const Character& c = player(); int n = 0;
  if(slot != EQ_HAND_R && eq_get(c, slot) >= 0) out[n++] = -1;   // -1 = retirer
  for(int i = 0; i < c.inv.n && n < 16; ++i){
    int it = c.inv.slots[i].item;
    if(slot_accepts(slot, it)) out[n++] = it;
  }
  return n;
}

// Deplacement spatial sur la poupee : emplacement le plus proche dans la direction
static int move_slot(int from, int dx, int dy){
  int best = from, bestScore = 1 << 30;
  int fx = sx(from), fy = sy(from);
  for(int s = 0; s < EQ_COUNT; ++s){
    if(s == from) continue;
    int ddx = sx(s) - fx, ddy = sy(s) - fy;
    int along = dx ? ddx * dx : ddy * dy;           // distance dans la direction demandee
    int across = dx ? abs(ddy) : abs(ddx);          // ecart lateral (penalise)
    if(along <= 0) continue;
    int score = along + across * 2;
    if(score < bestScore){ bestScore = score; best = s; }
  }
  return best;
}

static void equip_update(uint32_t p){
  Character& c = player();
  if(s_pick){
    int L[17]; int n = pick_list(s_slot, L);
    if(p & gb::BTN_B){ s_pick = false; return; }
    if(n == 0){ s_pick = false; return; }
    if(s_psel >= n) s_psel = n - 1;
    if(p & gb::BTN_DOWN) s_psel = (s_psel + 1) % n;
    if(p & gb::BTN_UP)   s_psel = (s_psel + n - 1) % n;
    if(p & gb::BTN_A){
      int it = L[s_psel]; s_buf[0] = 0;
      bool ok = (it < 0) ? unequip_slot(c, s_slot, s_buf, sizeof(s_buf)) : equip_item(c, it, s_slot, s_buf, sizeof(s_buf));
      s_msg = s_buf[0] ? s_buf : nullptr; s_pick = false;   // message de reussite ou de refus (arc a deux mains)
      if(ok) audio::sfx_confirm(); else audio::sfx_error();
    }
    return;
  }
  int old = s_slot;
  if(p & gb::BTN_LEFT)  s_slot = move_slot(s_slot, -1, 0);
  if(p & gb::BTN_RIGHT) s_slot = move_slot(s_slot,  1, 0);
  if(p & gb::BTN_UP)    s_slot = move_slot(s_slot, 0, -1);
  if(p & gb::BTN_DOWN)  s_slot = move_slot(s_slot, 0,  1);
  if(s_slot != old) s_msg = nullptr;
  if(p & gb::BTN_A){
    int L[17];
    if(pick_list(s_slot, L) == 0){ s_msg = "Aucun objet du sac ne va ici."; audio::sfx_error(); }
    else { s_pick = true; s_psel = 0; s_msg = nullptr; }
  }
}

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
  s_buf[0] = 0;
  if(is_equippable(it)){
    // A sur un objet porte (hors arme) : on le retire ; sinon on l'equipe a sa place
    bool ok = false;
    if(is_equipped(c, it) && it != c.weapon){
      for(int s = 1; s < EQ_COUNT; ++s) if(c.equip[s] == it){ ok = unequip_slot(c, s, s_buf, sizeof(s_buf)); break; }
    } else ok = equip_item(c, it, -1, s_buf, sizeof(s_buf));
    s_msg = s_buf[0] ? s_buf : nullptr;
    if(ok) audio::sfx_confirm(); else audio::sfx_error();
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
  if(p & gb::BTN_B){ if(s_pick){ s_pick = false; return; } close(); return; }
  if(p & (gb::BTN_L1 | gb::BTN_R1)){                // onglet precedent / suivant
    s_tab = (s_tab + ((p & gb::BTN_R1) ? 1 : TAB_N - 1)) % TAB_N; s_pick = false; s_msg = nullptr; return; }
  if(s_tab == TAB_EQUIP) equip_update(p);
  else if(s_tab == TAB_BAG) bag_update(p);
  else spell_update(p);
}

// --- Rendu -------------------------------------------------------------------
static const gb::Color TXT = gb::rgb(235,225,190), DIM = gb::rgb(170,160,132), LINE = gb::rgb(70,56,32);
static void frame(int x, int y, int w, int h, gb::Color c){
  gb::fill_rect(x, y, w, 2, c); gb::fill_rect(x, y + h - 2, w, 2, c);
  gb::fill_rect(x, y, 2, h, c); gb::fill_rect(x + w - 2, y, 2, h, c);
}
static void bar(int x, int y, int w, int v, int vmax, gb::Color c){
  gb::fill_rect(x, y, w, 5, gb::rgb(50,40,28));
  if(vmax > 0 && v > 0){ int f = w * (v > vmax ? vmax : v) / vmax; gb::fill_rect(x, y, f, 5, c); }
}
static void hero_x2(int x, int y){                  // portrait : sprite de face, double taille
  const spr::Sprite& s = hero_sprite(0,1);
  for(int j = 0; j < s.h; ++j) for(int i = 0; i < s.w; ++i){
    uint16_t c = s.px[j * s.w + i]; if(c != spr::KEY) gb::fill_rect(x + i*2, y + j*2, 2, 2, c);
  }
}

// Comparaison : affiche les bonus qui changent ("Arm 3>4"), vert si mieux, rouge si moins bien
static void draw_compare(int x, int y, const EqStats& a, const EqStats& b){
  const char* lab[4] = { trc("Arm"), "CO", trc("Dég"), "AU" };
  int va[4] = { a.arm, a.co + a.hit, a.dmg, a.au }, vb[4] = { b.arm, b.co + b.hit, b.dmg, b.au };
  bool any = false;
  for(int k = 0; k < 4; ++k){
    if(va[k] == vb[k]) continue;
    char t[32]; snprintf(t, sizeof(t), (k == 0) ? "%s %d>%d" : "%s %+d>%+d", lab[k], va[k], vb[k]);
    ui::text(x, y, t, vb[k] > va[k] ? gb::rgb(130,220,120) : gb::rgb(235,110,95));
    x += ui::text_w(t) + 12; any = true;
  }
  if(!any) ui::text(x, y, trc("Aucun changement."), DIM);
}

// Silhouette de la poupee (en attendant une illustration dediee)
static void doll_silhouette(){
  const gb::Color S = gb::rgb(52,41,26);
  gb::fill_rect(208,56,16,18,S);                       // tete
  gb::fill_rect(212,74,8,6,S);                         // cou
  gb::fill_rect(198,80,36,34,S);                       // buste
  gb::fill_rect(186,82,12,34,S); gb::fill_rect(234,82,12,34,S);   // bras
  gb::fill_rect(182,114,12,8,S); gb::fill_rect(238,114,12,8,S);   // mains
  gb::fill_rect(202,114,12,44,S); gb::fill_rect(218,114,12,44,S); // jambes
}

static void slot_box(int s, bool selected){
  const Character& c = player(); int x = sx(s), y = sy(s), it = eq_get(c, s);
  bool blocked = (s == EQ_HAND_L && is_two_handed(c.weapon));
  gb::fill_rect(x, y, 26, 26, blocked ? gb::rgb(40,30,24) : gb::rgb(30,24,14));
  frame(x, y, 26, 26, it >= 0 ? gb::rgb(150,120,60) : LINE);
  if(it >= 0) item_ui::draw_icon(it, x + 1, y + 1, false);
  else if(blocked) item_ui::draw_icon(c.weapon, x + 1, y + 1, false);   // l'arc occupe les deux mains
  if(selected) frame(x - 1, y - 1, 28, 28, gb::rgb(255,255,255));
}

void render(){
  if(!s_open) return;
  const Character& c = player();
  const gb::Color GOLDC = gb::rgb(200,170,90);
  EqStats st = eq_stats(c);
  gb::fill_rect(6,30,308,206,gb::rgb(20,16,10));
  gb::fill_rect(6,30,308,2,GOLDC); gb::fill_rect(6,234,308,2,GOLDC);
  gb::fill_rect(118,34,1,136,LINE);                    // separation des colonnes

  // Colonne gauche : identite, jauges, caracteristiques (bonus d'equipement compris, en vert)
  hero_x2(12,36);
  ui::gold(42,36,c.name,true);
  char ln[48];
  snprintf(ln,sizeof(ln),trc("Niv. %d"),c.level);   ui::text(42,53,ln,TXT);
  snprintf(ln,sizeof(ln),trc("%d or"),c.gold);       ui::text(42,68,ln,gb::rgb(245,215,95));
  int pv = c.VIT - c.BL; if(pv < 0) pv = 0;
  snprintf(ln,sizeof(ln),"PV %d/%d",pv,c.VIT);       ui::text(12,84,ln,TXT);  bar(12,99,100,pv,c.VIT,gb::rgb(200,60,50));
  snprintf(ln,sizeof(ln),"PM %d/%d",c.mana,c.manaMax); ui::text(12,106,ln,TXT); bar(12,121,100,c.mana,c.manaMax,gb::rgb(80,120,230));
  const gb::Color UP = gb::rgb(130,220,120);
  snprintf(ln,sizeof(ln),"IN %d",c.IN); ui::text(12,128,ln,TXT);
  snprintf(ln,sizeof(ln),"CO %d",c.CO + st.co); ui::text(62,128,ln,st.co ? UP : TXT);
  snprintf(ln,sizeof(ln),"MA %d",c.MA); ui::text(12,142,ln,TXT);
  snprintf(ln,sizeof(ln),"AU %d",c.AU + st.au); ui::text(62,142,ln,st.au ? UP : TXT);
  snprintf(ln,sizeof(ln),trc("Armure %d"),st.arm); ui::text(12,156,ln,st.arm ? UP : TXT);

  // Onglets
  const char* T[TAB_N] = { trc("Équipement"), trc("Sac"), trc("Sorts") };
  int tx = 124;
  for(int k = 0; k < TAB_N; ++k){
    int w = ui::text_w(T[k]) + 10;
    gb::fill_rect(tx,34,w,16,s_tab == k ? gb::rgb(60,45,20) : gb::rgb(30,24,14));
    if(s_tab == k) ui::gold(tx + 5,35,T[k],true); else ui::text(tx + 5,35,T[k],DIM);
    tx += w + 3;
  }

  const char* name = nullptr; const char* desc = nullptr; char nameBuf[64];
  int cmpItem = -1, cmpSlot = -1;   // objet a comparer avec l'equipement actuel
  if(s_tab == TAB_EQUIP){
    if(!s_pick){
      if(!item_ui::draw_doll(216,52,116)) doll_silhouette();   // illustration si fournie
      for(int s = 0; s < EQ_COUNT; ++s) slot_box(s, s == s_slot);
      int it = eq_get(c, s_slot);
      if(it >= 0) snprintf(nameBuf,sizeof(nameBuf),"%s : %s",trc(eq_slot_name(s_slot)),trc(ITEMS[it].name));
      else snprintf(nameBuf,sizeof(nameBuf),"%s : %s",trc(eq_slot_name(s_slot)),trc("vide"));
      name = nameBuf; desc = it >= 0 ? trc(item_desc(it)) : nullptr;
    } else {
      // Liste des objets possibles pour l'emplacement
      int L[17]; int n = pick_list(s_slot, L);
      if(s_psel >= n) s_psel = n ? n - 1 : 0;
      ui::gold(126,54,trc(eq_slot_name(s_slot)),true);
      const int VIS = 4; int top = s_psel >= VIS ? s_psel - VIS + 1 : 0;
      for(int r = 0; r < VIS && top + r < n; ++r){
        int it = L[top + r], y = 72 + r*25;
        if(top + r == s_psel) gb::fill_rect(124,y - 1,186,25,gb::rgb(60,45,20));
        if(it < 0){ ui::text(156,y + 4,trc("Retirer"),TXT); continue; }
        item_ui::draw_icon(it,128,y,false);
        ui::text(156,y + 4,trc(ITEMS[it].name),is_equipped(c,it) ? gb::rgb(245,215,95) : TXT);
      }
      int it = n ? L[s_psel] : -1;
      if(it >= 0){ name = trc(ITEMS[it].name); desc = trc(item_desc(it)); cmpItem = it; cmpSlot = s_slot; }
      else { name = trc("Retirer"); }
    }
  } else if(s_tab == TAB_BAG){
    int n = c.inv.n;
    if(n == 0){ ui::text(GX,GY + 8,trc("Le sac est vide."),DIM); }
    else {
      if(s_sel >= n) s_sel = n - 1;
      int row = s_sel / COLS, top = row >= ROWS ? row - ROWS + 1 : 0;
      for(int r = 0; r < ROWS; ++r) for(int col = 0; col < COLS; ++col){
        int idx = (top + r) * COLS + col; int x = GX + col*CELL, y = GY + r*CELL;
        gb::fill_rect(x,y,CELL - 2,CELL - 2,gb::rgb(34,27,16));
        if(idx >= n) continue;
        const ItemStack& is = c.inv.slots[idx];
        item_ui::draw_icon(is.item,x + 1,y + 1,false);
        if(is.qty > 1){ char q[12]; snprintf(q,sizeof(q),"%d",(int)is.qty); int tw = ui::text_w(q);
          gb::fill_rect(x + CELL - 3 - tw,y + CELL - 13,tw + 1,11,gb::rgb(20,16,10)); ui::text(x + CELL - 3 - tw,y + CELL - 16,q,TXT); }
        if(is_equipped(c,is.item)){                     // objet porte : cadre dore + "E"
          frame(x,y,CELL - 2,CELL - 2,gb::rgb(245,215,95));
          gb::fill_rect(x + 2,y + 2,9,11,gb::rgb(20,16,10)); ui::text(x + 3,y,"E",gb::rgb(245,215,95));
        }
        if(idx == s_sel) frame(x - 1,y - 1,CELL,CELL,gb::rgb(255,255,255));
      }
      if(top > 0) ui::text(300,GY - 4,"^",DIM);
      if((top + ROWS) * COLS < n) ui::text(300,GY + ROWS*CELL - 12,"v",DIM);
      const ItemStack& is = c.inv.slots[s_sel];
      bool eq = is_equipped(c,is.item);
      snprintf(nameBuf,sizeof(nameBuf),is.qty > 1 ? "%s x%d" : "%s",trc(ITEMS[is.item].name),(int)is.qty);
      if(eq){ size_t l = strlen(nameBuf); snprintf(nameBuf + l,sizeof(nameBuf) - l," (%s)",trc("équipé")); }
      name = nameBuf; desc = trc(item_desc(is.item));
      if(is_equippable(is.item) && !eq){ cmpItem = is.item; cmpSlot = -1; }
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

  // Bas : nom, comparaison ou resultat, description, aide
  gb::fill_rect(10,172,300,1,LINE);
  if(name) ui::gold(12,175,name,true);
  int y = 192;
  if(s_msg){ ui::text(12,y,trc(s_msg),gb::rgb(245,215,95)); y += 14; }
  else if(cmpItem >= 0){ draw_compare(12,y,st,eq_stats_if(c,cmpItem,cmpSlot)); y += 14; }
  if(desc && y < 220){ char W[2][64]; int k = item_ui::wrap_text(desc,296,W,(220 - y) / 14);
    for(int i = 0; i < k; ++i) ui::text(12,y + i*14,W[i],gb::rgb(185,225,175)); }
  static const char* const HINT[TAB_N] = {
    "A: choisir  L1/R1: onglet  B: fermer", "A: équiper/utiliser  L1/R1: onglet  B: fermer", "A: lancer  L1/R1: onglet  B: fermer" };
  ui::text(12,220,trc(s_pick ? "A: équiper  B: annuler" : HINT[s_tab]),DIM);
}

}}
