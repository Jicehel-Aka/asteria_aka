// ASTERIA (Gamebuino AKA) - Carte du monde (voir worldmap.h).
#include "scene/worldmap.h"
#include "scene/journal.h"
#include "scene/world_scene.h"
#include "ui/text.h"
#include "platform/gb_port.h"
#include "generated/asteria_quest.h"
#include "generated/asteria_maps.h"
#include "gamestate.h"
#include "progress.h"
#include "content_i18n.h"
#include "player.h"
#include <cstdio>
#include <cstdlib>
namespace asteria { namespace worldmap {

static const char* BG_PATH = "/sdcard/ASTERIA/CARTE.BMP";

// Zones de la carte : position a l'ecran, nom, type de pictogramme
enum Kind { K_TOWN, K_FOREST, K_MINE, K_CITY, K_ROAD, K_TEMPLE, K_PLAIN, K_STONES };
struct Zone { int8_t loc; int16_t x, y; uint8_t kind; const char* name; const char* desc; };
static const Zone Z[] = {
  {22, 196,186, K_PLAIN,  "Grande Plaine",      "Là où la Brèche s'est refermée."},
  { 0, 196,152, K_TOWN,   "Valbois",            "Village de bûcherons, au bord de la forêt."},
  { 8, 182,114, K_FOREST, "Forêt de Brume",     "Herbes, champignons... et loups."},
  { 9, 236, 82, K_MINE,   "Vieille Mine",       "Galeries abandonnées, envahies de gobelins."},
  {10, 196, 50, K_CITY,   "Grand-Castel",       "Cité fortifiée : Guilde, temple, bibliothèque."},
  {21, 264, 34, K_TEMPLE, "Temple Oublié",      "Scellé depuis des âges."},
  {20, 110, 88, K_ROAD,   "Route Royale",       "Grand chemin des marchands... et des brigands."},
  {23,  56, 54, K_STONES, "Vallée des Pierres", "Des monolithes gravés murmurent entre les rochers."} };
static const int ZN = (int)(sizeof(Z) / sizeof(Z[0]));
// Positions sur l'illustration CARTE.BMP (320x240), dans l'ordre de Z[]
static const int16_t BMP_XY[][2] = { {202,173}, {190,138}, {175,100}, {265,78}, {190,40}, {290,32}, {58,88}, {40,46} };
static int16_t PX[8], PY[8];   // positions utilisees (parchemin dessine ou illustration)
// Chemins (indices dans Z), d'apres les passages entre les cartes du jeu
static const uint8_t EDGES[][2] = { {0,1}, {1,2}, {2,3}, {3,4}, {4,5}, {4,6}, {6,2}, {6,7} };
static const int EN = (int)(sizeof(EDGES) / sizeof(EDGES[0]));

static bool s_open = false, s_bmp = false;
static int  s_sel = 1, s_quest = -1;

static int here(){                                  // zone du joueur (interieurs 1..7 -> Valbois)
  int loc = data::MAPS[world_scene().map].location;
  return (loc >= 1 && loc <= 7) ? 0 : loc;
}
static bool known(int i){
  int it = item_index("OBJ_0017");                  // Carte regionale : toutes les zones nommees
  return zone_discovered(Z[i].loc) || Z[i].loc == here() || (it >= 0 && player().inv.has(it, 1));
}
static int zone_of(int loc){ for(int i = 0; i < ZN; ++i) if(Z[i].loc == loc) return i; return -1; }

void open(int q){
  s_open = true; s_quest = q;
  int h = zone_of(here()); s_sel = h >= 0 ? h : 1;
  s_bmp = gb::blit_bmp(BG_PATH);                    // illustration facultative (testee une fois)
  for(int i = 0; i < ZN; ++i){ PX[i] = s_bmp ? BMP_XY[i][0] : Z[i].x; PY[i] = s_bmp ? BMP_XY[i][1] : Z[i].y; }
}
bool is_open(){ return s_open; }
void close(){ s_open = false; }

void update(uint32_t p){
  if(p & (gb::BTN_B | gb::BTN_MENU | gb::BTN_A)){ close(); return; }
  int dx = (p & gb::BTN_RIGHT) ? 1 : (p & gb::BTN_LEFT) ? -1 : 0;
  int dy = (p & gb::BTN_DOWN) ? 1 : (p & gb::BTN_UP) ? -1 : 0;
  if(!dx && !dy) return;
  // zone la plus proche dans la direction demandee
  int best = s_sel, bs = 1 << 30;
  for(int i = 0; i < ZN; ++i){
    if(i == s_sel) continue;
    int ddx = PX[i] - PX[s_sel], ddy = PY[i] - PY[s_sel];
    int along = dx ? ddx * dx : ddy * dy, across = dx ? abs(ddy) : abs(ddx);
    if(along <= 0) continue;
    int sc = along + across * 2; if(sc < bs){ bs = sc; best = i; }
  }
  s_sel = best;
}

// --- Rendu -------------------------------------------------------------------
static void disc(int cx, int cy, int r, gb::Color c){
  for(int y = -r; y <= r; ++y){ int w = 0; while((w + 1) * (w + 1) + y * y <= r * r) ++w; gb::fill_rect(cx - w, cy + y, 2 * w + 1, 1, c); }
}
static void tri(int cx, int by, int h, gb::Color c){   // montagne : triangle plein
  for(int y = 0; y < h; ++y){ int w = y * 3 / 4; gb::fill_rect(cx - w, by - h + y, 2 * w + 1, 1, c); }
}
static void dashed(int x0, int y0, int x1, int y1, gb::Color c, int on){
  int n = abs(x1 - x0) > abs(y1 - y0) ? abs(x1 - x0) : abs(y1 - y0); if(n == 0) return;
  for(int k = 0; k <= n; ++k){
    if((k / on) & 1) continue;
    int x = x0 + (x1 - x0) * k / n, y = y0 + (y1 - y0) * k / n;
    gb::fill_rect(x, y, 2, 2, c);
  }
}
static uint32_t hash(uint32_t v){ v ^= v >> 16; v *= 0x7feb352d; v ^= v >> 15; v *= 0x846ca68b; return v ^ (v >> 16); }

static void parchment(){
  const gb::Color P = gb::rgb(214,192,142), P2 = gb::rgb(198,174,124), INK = gb::rgb(120,92,58);
  gb::fill_rect(0,0,320,240,P);
  for(int i = 0; i < 260; ++i){                     // grain du papier (fixe)
    uint32_t h = hash(i * 977 + 13); gb::fill_rect(h % 320, (h >> 9) % 240, 2 + (h >> 20) % 3, 1, P2);
  }
  // riviere : de la Vallee vers le sud-est, passe a l'ouest de Valbois
  int rx = 30, ry = 20;
  for(int k = 0; k < 230; ++k){ int x = rx + k * 120 / 230 + (int)((hash(k / 12) % 7)) - 3, y = ry + k; gb::fill_rect(x, y, 4, 1, gb::rgb(110,150,175)); }
  // relief au nord (mine, temple) et collines de la vallee
  const gb::Color M = gb::rgb(150,120,86), MS = gb::rgb(176,150,110);
  static const int16_t MNT[][2] = { {222,80},{248,74},{270,86},{288,62},{240,40},{282,30},{300,48},{40,44},{70,40},{24,70} };
  for(auto& m : MNT){ tri(m[0], m[1], 16, M); tri(m[0] - 2, m[1], 10, MS); }
  // foret autour de la Foret de Brume
  for(int i = 0; i < 26; ++i){ uint32_t h = hash(i * 31 + 7); int x = 140 + h % 70, y = 96 + (h >> 8) % 40; disc(x, y, 4, gb::rgb(84,112,64)); disc(x - 1, y - 1, 2, gb::rgb(110,140,80)); }
  // cadre
  gb::fill_rect(0,0,320,3,INK); gb::fill_rect(0,237,320,3,INK); gb::fill_rect(0,0,3,240,INK); gb::fill_rect(317,0,3,240,INK);
  // rose des vents
  const gb::Color R = gb::rgb(140,100,60);
  gb::fill_rect(296,160,1,22,R); gb::fill_rect(286,170,21,1,R); ui::text(293,146,"N",R);
}

static void picto(int i, bool k){
  const Zone& z = Z[i]; int x = PX[i], y = PY[i];
  const gb::Color INK = gb::rgb(70,50,30);
  if(!k){ disc(x, y, 7, gb::rgb(170,150,112)); ui::text(x - 3, y - 8, "?", INK); return; }
  if(s_bmp){ disc(x, y, 5, INK); disc(x, y, 3, gb::rgb(245,215,95)); return; }   // illustration : simple repere
  disc(x, y, 8, INK);
  gb::Color c;
  switch(z.kind){
    case K_TOWN: c = gb::rgb(190,120,70); break;  case K_CITY: c = gb::rgb(200,190,170); break;
    case K_FOREST: c = gb::rgb(90,140,70); break; case K_MINE: c = gb::rgb(120,110,100); break;
    case K_TEMPLE: c = gb::rgb(170,80,90); break; case K_STONES: c = gb::rgb(150,150,170); break;
    case K_ROAD: c = gb::rgb(200,170,100); break; default: c = gb::rgb(170,190,110); break;
  }
  disc(x, y, 6, c);
  if(z.kind == K_TOWN || z.kind == K_CITY){ gb::fill_rect(x - 3, y - 1, 7, 4, INK); tri(x, y - 1, 4, INK); }   // maison
  if(z.kind == K_MINE || z.kind == K_TEMPLE){ gb::fill_rect(x - 3, y - 2, 6, 5, INK); }                         // entree
  if(z.kind == K_FOREST){ disc(x, y - 1, 3, gb::rgb(40,80,40)); gb::fill_rect(x, y + 2, 1, 3, INK); }         // arbre
}
static void star(int x, int y, gb::Color c){
  gb::fill_rect(x - 1, y - 5, 3, 11, c); gb::fill_rect(x - 5, y - 1, 11, 3, c);
  gb::fill_rect(x - 3, y - 3, 7, 7, c);
}

void render(){
  if(!s_open) return;
  if(!(s_bmp && gb::blit_bmp(BG_PATH))) parchment();
  const gb::Color INK = gb::rgb(70,50,30), ROAD = gb::rgb(120,86,50), FAINT = gb::rgb(176,156,118);
  for(int e = 0; e < EN && !s_bmp; ++e){            // l'illustration a deja ses routes
    int a = EDGES[e][0], b = EDGES[e][1];
    bool k = known(EDGES[e][0]) && known(EDGES[e][1]);
    dashed(PX[a], PY[a], PX[b], PY[b], k ? ROAD : FAINT, k ? 4 : 2);
  }
  int target = -1;
  if(s_quest >= 0 && quest_status(s_quest) == 1){ int loc = -1; journal::next_objective(s_quest, &loc); target = zone_of(loc); }
  int me = zone_of(here());
  uint32_t t = gb::millis();
  for(int i = 0; i < ZN; ++i){
    bool k = known(i); picto(i, k);
    if(k){ const char* nm = trc(Z[i].name); int w = ui::text_w(nm);
      int tx = PX[i] - w / 2; if(tx < 6) tx = 6; if(tx + w > 314) tx = 314 - w;
      int ty = PY[i] + 9; if(ty + 14 > 196) ty = PY[i] - 24;   // pres du bandeau du bas : nom au-dessus
      gb::fill_rect(tx - 2, ty + 1, w + 4, 13, gb::rgb(232,214,168)); ui::text(tx, ty, nm, INK); }
  }
  if(target >= 0) star(PX[target] + 11, PY[target] - 9, (t / 300) & 1 ? gb::rgb(245,200,60) : gb::rgb(200,140,30));
  if(me >= 0 && ((t / 250) & 1)){ disc(PX[me], PY[me], 4, gb::rgb(200,40,40)); }
  // curseur
  const Zone& s = Z[s_sel]; int cx = PX[s_sel], cy = PY[s_sel];
  gb::fill_rect(cx - 11, cy - 11, 23, 2, gb::rgb(255,255,255)); gb::fill_rect(cx - 11, cy + 10, 23, 2, gb::rgb(255,255,255));
  gb::fill_rect(cx - 11, cy - 11, 2, 23, gb::rgb(255,255,255)); gb::fill_rect(cx + 10, cy - 11, 2, 23, gb::rgb(255,255,255));

  // Bandeau : titre, zone selectionnee, prochain objectif
  gb::fill_rect(0,0,320,18,gb::rgb(20,16,10)); ui::gold(6,1,trc("Carte du monde"),true);
  char ln[96];
  gb::fill_rect(0,196,320,44,gb::rgb(20,16,10));
  if(known(s_sel)){
    snprintf(ln,sizeof(ln),"%s%s",s_sel == me ? "» " : "",trc(s.name));
    ui::gold(6,197,ln,true); ui::text(6,211,trc(s.desc),gb::rgb(235,225,190));
  } else ui::text(6,204,trc("Contrée inexplorée."),gb::rgb(170,160,132));
  if(target >= 0){ snprintf(ln,sizeof(ln),"* %s",trc(journal::next_objective(s_quest,nullptr))); ui::text(6,225,ln,gb::rgb(245,215,95)); }
  else ui::text(6,225,trc("Flèches : lieux   B : fermer"),gb::rgb(170,160,132));
}

}}
