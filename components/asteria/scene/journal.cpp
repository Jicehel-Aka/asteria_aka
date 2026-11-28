// ASTERIA (Gamebuino AKA) - Journal de quetes (voir journal.h).
#include "scene/journal.h"
#include "ui/text.h"
#include "platform/gb_port.h"
#include "generated/asteria_quest.h"
#include "generated/asteria_gen.h"
#include "gamestate.h"
#include "progress.h"
#include "content_i18n.h"
#include "player.h"
#include <cstdio>
namespace asteria { namespace journal {

static bool s_open = false;
static int  s_tab = 0;     // 0 = en cours, 1 = terminees
static int  s_sel = 0;

// Lieu de chaque objectif (identifiant de zone, voir loc_name de world_scene) ;
// 0 Valbois, 8 Foret de Brume, 9 Vieille Mine, 10 Grand-Castel, 20 Route Royale,
// 21 Antre / Temple Oublie, 22 Grande Plaine, 23 Vallee des Pierres.
static const int8_t OBJ_LOC[quest::QUEST_COUNT][6] = {
  { 0, 0, 0},            // Bienvenue a Valbois
  { 9, 0},               // Les Outils perdus : dans la mine, puis a la forge de Doran
  { 8, 8},               // Les Traces etranges
  { 8},                  // Les empreintes
  { 8, 8},               // Les plantes de soeur Agnes (cueillette en foret)
  { 8, 9},               // Le tissu dechire
  { 9, 9, 9, 9, 9},      // La Vieille Mine
  {10,10,10},            // Le symbole oublie (Maitre Albin, bibliotheque)
  {10,10,10,10},         // Les contrats de la Guilde
  {20,20},               // Les marchandises disparues
  {20,20},               // La caravane silencieuse
  {20},                  // Avis de recherche
  {20},                  // Une roue cassee
  {23,23,23,23},         // Les Pierres Murmurantes
  {21,21,21,21,21,21},   // Le Temple Oublie
  {10,10,10},            // Le Collier d'hermine
  { 0, 0},               // L'Anneau de l'Aube
  {22,22, 0} };          // Le Passage d'Asterion

static const char* zone_name(int loc){
  switch(loc){
    case 0: return "Valbois";          case 8: return "Forêt de Brume";
    case 9: return "Vieille Mine";     case 10: return "Grand-Castel";
    case 20: return "Route Royale";    case 21: return "Temple Oublié";
    case 22: return "Grande Plaine";   case 23: return "Vallée des Pierres";
    default: return "";
  }
}

static int count_of(const char* id){
  int it = item_index(id); if(it < 0) return 0;
  int k = player().inv.find(it); return k < 0 ? 0 : player().inv.slots[k].qty;
}
static int pierres(){ return flag_get("pm6") + flag_get("pm12") + flag_get("pm22") + flag_get("pm36"); }

// Objectif o de la quete q accompli ? Deduit de l'etat reel du jeu (la plupart des
// quetes ne font pas avancer quest_obj) ; repli sur quest_obj pour les autres.
static bool obj_done(int q, int o){
  if(quest_status(q) == 2) return true;
  switch(q){
    case 0:  return o == 0 ? flag_get("valbois_intro") : o == 1 ? flag_get("vu_maire") : count_of("OBJ_0006") > 0;
    case 1:  return o == 0 ? (count_of("OBJ_0010") > 0 || flag_get("doran_resolu")) : flag_get("doran_resolu");
    case 4:  return o == 0 ? count_of("OBJ_0034") >= 5 : count_of("OBJ_0035") >= 3;
    case 5:  return o == 0 ? flag_get("piste_mine") : zone_discovered(9);
    case 6:  return o < quest_obj(6) || (o >= 3 && flag_get("boss_mine"));
    case 7:  return o == 0;                                   // la quete s'ouvre quand on presente la pierre
    case 8:  { static const char* F[4] = {"c_pli","c_herbes","c_aumone","q8_fait"}; return flag_get(F[o]); }
    case 9:  return flag_get("route_intro");
    case 11: return flag_get("boss_corbeau");
    case 12: return flag_get("q12_fait");
    case 13: return o == 0 ? flag_get("vallee_intro") : o < 3 ? pierres() > 0 : pierres() == 4;
    case 14: { static const char* F[6] = {"antre_intro","vu_gardien","vu_gardien","boss_gardien","vu_coeur","boss_coeur"}; return flag_get(F[o]); }
    case 15: return o == 0 ? flag_get("vu_meluve") : o == 1 ? (flag_get("a_collier") || flag_get("collier_pris")) : flag_get("q15_fait");
    case 16: return o == 0 ? true : flag_get("anneau_recu");
    case 17: return o == 0 ? true : o == 1 ? flag_get("vu_voyageur") : flag_get("valbois_intro");
    default: return o < quest_obj(q);
  }
}
// Compteur affiche apres certains objectifs ("3/5")
static bool obj_count(int q, int o, int& have, int& need){
  if(quest_status(q) == 2) return false;
  if(q == 4){ have = o == 0 ? count_of("OBJ_0034") : count_of("OBJ_0035"); need = o == 0 ? 5 : 3; }
  else if(q == 13 && o == 3){ have = pierres(); need = 4; }
  else return false;
  if(have > need) have = need;
  return true;
}

const char* next_objective(int q, int* loc){
  const quest::Quest& Q = quest::QUEST[q];
  for(int o = 0; o < Q.nobj; ++o) if(!obj_done(q, o)){ if(loc) *loc = OBJ_LOC[q][o]; return Q.obj[o]; }
  if(loc) *loc = OBJ_LOC[q][Q.nobj - 1];
  return Q.obj[Q.nobj - 1];
}

static int list(int tab, int* out){
  int n = 0;
  // principales d'abord, puis secondaires ; dans l'ordre du recit
  for(int pass = 0; pass < 2; ++pass)
    for(int i = 0; i < quest::QUEST_COUNT; ++i){
      int st = quest_status(i);
      if(st == 0 || (tab == 0) != (st == 1)) continue;
      if(quest::QUEST[i].main != (pass == 0)) continue;
      out[n++] = i;
    }
  return n;
}

void open(){ s_open = true; s_tab = 0; s_sel = 0; int L[quest::QUEST_COUNT]; if(list(0, L) == 0) s_tab = 1; }
bool is_open(){ return s_open; }
void close(){ s_open = false; }

void update(uint32_t p){
  if(p & (gb::BTN_B | gb::BTN_MENU)){ close(); return; }
  if(p & (gb::BTN_LEFT | gb::BTN_RIGHT | gb::BTN_L1 | gb::BTN_R1)){ s_tab ^= 1; s_sel = 0; return; }
  int L[quest::QUEST_COUNT]; int n = list(s_tab, L); if(n == 0) return;
  if(p & gb::BTN_DOWN) s_sel = (s_sel + 1) % n;
  if(p & gb::BTN_UP)   s_sel = (s_sel + n - 1) % n;
}

static void check(int x, int y, gb::Color c){   // coche dessinee (la police n'a pas de glyphe)
  gb::fill_rect(x, y + 5, 2, 2, c); gb::fill_rect(x + 2, y + 7, 2, 2, c);
  gb::fill_rect(x + 4, y + 5, 2, 2, c); gb::fill_rect(x + 6, y + 3, 2, 2, c); gb::fill_rect(x + 8, y + 1, 2, 2, c);
}

void render(){
  if(!s_open) return;
  const gb::Color GOLDC = gb::rgb(200,170,90), TXT = gb::rgb(235,225,190), DIM = gb::rgb(170,160,132),
                  OK = gb::rgb(130,210,120), CUR = gb::rgb(245,215,95), LINE = gb::rgb(70,56,32);
  gb::fill_rect(6,30,308,206,gb::rgb(20,16,10));
  gb::fill_rect(6,30,308,2,GOLDC); gb::fill_rect(6,234,308,2,GOLDC);
  ui::gold(14,35,trc("Journal de quêtes"),true);

  int L0[quest::QUEST_COUNT], L1[quest::QUEST_COUNT];
  int n0 = list(0, L0), n1 = list(1, L1);
  char t0[32], t1[32];
  snprintf(t0,sizeof(t0),trc("En cours (%d)"),n0); snprintf(t1,sizeof(t1),trc("Terminées (%d)"),n1);
  int w0 = ui::text_w(t0) + 10, w1 = ui::text_w(t1) + 10, x0 = 12, x1 = x0 + w0 + 3;   // onglets sous le titre
  gb::fill_rect(x0,52,w0,16,s_tab == 0 ? gb::rgb(60,45,20) : gb::rgb(30,24,14));
  gb::fill_rect(x1,52,w1,16,s_tab == 1 ? gb::rgb(60,45,20) : gb::rgb(30,24,14));
  ui::text(x0 + 5,53,t0,s_tab == 0 ? CUR : DIM); ui::text(x1 + 5,53,t1,s_tab == 1 ? CUR : DIM);

  const int* L = s_tab ? L1 : L0; int n = s_tab ? n1 : n0;
  if(n == 0){ ui::text(14,76,trc(s_tab ? "Aucune quête terminée." : "Aucune quête active."),DIM); }
  else {
    if(s_sel >= n) s_sel = n - 1;
    const int VIS = 4; int top = s_sel >= VIS ? s_sel - VIS + 1 : 0;
    for(int r = 0; r < VIS && top + r < n; ++r){
      int q = L[top + r], y = 73 + r*16;
      if(top + r == s_sel) gb::fill_rect(10,y - 1,300,16,gb::rgb(60,45,20));
      bool main = quest::QUEST[q].main;
      ui::text(14,y,main ? "»" : "·",main ? CUR : DIM);           // » principale, · secondaire
      ui::text(28,y,trc(quest::QUEST[q].name),s_tab ? OK : TXT);
    }
    if(top > 0) ui::text(300,70,"^",DIM);
    if(top + VIS < n) ui::text(300,123,"v",DIM);

    // Detail de la quete selectionnee
    int q = L[s_sel]; const quest::Quest& Q = quest::QUEST[q];
    gb::fill_rect(10,139,300,1,LINE);
    int loc = -1; next_objective(q, &loc);
    char info[80];
    if(s_tab == 0 && loc >= 0) snprintf(info,sizeof(info),"%s  ·  %s %s",trc(Q.main ? "Quête principale" : "Quête secondaire"),trc("Lieu :"),trc(zone_name(loc)));
    else snprintf(info,sizeof(info),"%s",trc(Q.main ? "Quête principale" : "Quête secondaire"));
    ui::text(14,143,info,DIM);
    // objectifs accomplis + objectif en cours (les suivants restent caches)
    int last = Q.nobj - 1; for(int o = 0; o < Q.nobj; ++o) if(!obj_done(q, o)){ last = o; break; }
    const int OV = 4; int first = last - OV + 1; if(first < 0) first = 0;
    for(int o = first; o <= last; ++o){
      int y = 160 + (o - first)*14; bool d = obj_done(q, o);
      if(d) check(14,y + 2,OK); else ui::text(14,y,">",CUR);
      char ln[80]; int h, nd;
      if(obj_count(q, o, h, nd)) snprintf(ln,sizeof(ln),"%s (%d/%d)",trc(Q.obj[o]),h,nd); else snprintf(ln,sizeof(ln),"%s",trc(Q.obj[o]));
      ui::text(28,y,ln,d ? OK : CUR);
    }
    if(last < Q.nobj - 1 && !obj_done(q, last)) ui::text(300,202,"...",DIM);
  }
  ui::text(14,220,trc("Haut/Bas: quêtes  </>: onglet  B: retour"),DIM);
}

}}
