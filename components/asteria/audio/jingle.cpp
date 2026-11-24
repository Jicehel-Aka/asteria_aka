// ASTERIA (Gamebuino AKA) - Melodies et effets sonores (jingles) joues via le port audio.
#include "audio/jingle.h"
#include "platform/gb_port.h"
#include "audio/music.h"

namespace asteria { namespace audio {

// --- Table de notes (Hz) -----------------------------------------------------
enum { R=0 };                         // silence
#define A3 220
#define B3 247
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G5 784

struct Note { uint16_t f; uint16_t ms; };

// Thème de mort : lent, descendant, mineur (triangle = feutré).
static const Note DEATH[] = {
  {A4,320},{R,60},{G4,320},{F4,320},{E4,520},{R,140},
  {D4,360},{C4,360},{B3,360},{A3,820},{R,1},
};
// Fanfare de victoire (carré, claironnant).
static const Note VICTORY[] = {
  {C5,150},{E5,150},{G5,150},{R,40},{G5,130},{C5,130},{E5,130},{G5,420},{R,1},
};
// Montée de niveau : arpège rapide ascendant.
static const Note LEVELUP[] = {
  {C5,90},{D5,90},{E5,90},{G5,220},{R,1},
};
// Motif de titre, discret.
static const Note TITLE[] = {
  {E4,160},{G4,160},{C5,160},{E5,300},{R,1},
};

static void fx_update();              // sequenceur des effets (plus bas)
static const Note* g_seq = nullptr;
static int          g_len = 0;
static int          g_idx = 0;
static uint32_t     g_next = 0;       // date (ms) de déclenchement de la note g_idx
static bool         g_active = false;
static uint8_t      g_type = gb::TONE_SQUARE;

static void start(const Note* seq, int len, uint8_t type){
  g_seq=seq; g_len=len; g_idx=0; g_type=type;
  g_next=gb::millis(); g_active=true;
}

void init(){ gb::audio_init(); g_active=false; g_seq=nullptr; g_idx=0; }

void stop(){ g_active=false; g_seq=nullptr; gb::audio_stop(); }

bool playing(){ return g_active; }

void play(Tune t){
  if(t==TUNE_DEATH||t==TUNE_VICTORY||t==TUNE_LEVELUP) music::hold_for_jingle();   // la musique s'efface le temps du jingle
  switch(t){
    case TUNE_DEATH:   start(DEATH,   (int)(sizeof(DEATH)/sizeof(Note)),   gb::TONE_TRI);    break;
    case TUNE_VICTORY: start(VICTORY, (int)(sizeof(VICTORY)/sizeof(Note)), gb::TONE_SQUARE); break;
    case TUNE_LEVELUP: start(LEVELUP, (int)(sizeof(LEVELUP)/sizeof(Note)), gb::TONE_SQUARE); break;
    case TUNE_TITLE:   start(TITLE,   (int)(sizeof(TITLE)/sizeof(Note)),   gb::TONE_TRI);    break;
    default: g_active=false; break;
  }
}

void update(){
  fx_update();
  if(!g_active || !g_seq) return;
  uint32_t now=gb::millis();
  // Déclenche toutes les notes dont l'heure est passée (robuste aux frames sautées).
  while(g_active && now>=g_next){
    if(g_idx>=g_len){ g_active=false; break; }
    const Note& n=g_seq[g_idx++];
    if(n.f) gb::tone_music((float)n.f, 0.75f, n.ms, g_type);
    g_next += n.ms;
  }
}

// --- Effets courts (voix sfx) -----------------------------------------------
// Les effets de plusieurs notes passent par un 2e petit sequenceur, independant de
// celui des jingles : un son d'interface ne coupe jamais une melodie en cours.
struct SfxNote { uint16_t f; uint16_t ms; uint8_t type; uint8_t vol; };   // vol en %
static const SfxNote* s_fx = nullptr; static int s_fxLen = 0, s_fxIdx = 0; static uint32_t s_fxNext = 0;
static void fx_start(const SfxNote* seq, int len){ s_fx = seq; s_fxLen = len; s_fxIdx = 0; s_fxNext = gb::millis(); }
static void fx_update(){
  if(!s_fx) return;
  uint32_t now = gb::millis();
  while(s_fx && now >= s_fxNext){
    if(s_fxIdx >= s_fxLen){ s_fx = nullptr; break; }
    const SfxNote& n = s_fx[s_fxIdx++];
    if(n.f) gb::tone_sfx((float)n.f, n.vol / 100.f, n.ms, n.type);
    s_fxNext += n.ms;
  }
}
#define SQ gb::TONE_SQUARE
#define TR gb::TONE_TRI
#define SI gb::TONE_SINE
#define NZ gb::TONE_NOISE
static const SfxNote FX_PICKUP[] = { {784,45,SQ,30},{988,45,SQ,30},{1319,90,TR,35} };          // objet ramasse
static const SfxNote FX_CHEST[]  = { {196,40,NZ,30},{392,50,TR,35},{523,50,TR,35},{659,50,TR,35},{784,120,SI,40} };
static const SfxNote FX_DOOR[]   = { {150,35,NZ,25},{110,60,TR,35} };                          // porte / passage
static const SfxNote FX_BUY[]    = { {988,40,SQ,30},{1319,70,SQ,30} };                         // achat / vente
static const SfxNote FX_ERROR[]  = { {220,70,SQ,30},{0,20,SQ,0},{175,110,SQ,30} };             // action impossible
static const SfxNote FX_CRAFT[]  = { {330,40,NZ,20},{523,50,TR,35},{659,50,TR,35},{1047,110,SI,40} };
static const SfxNote FX_OPEN[]   = { {523,35,TR,28},{784,50,TR,28} };                          // ouverture d'un menu
static const SfxNote FX_SAVE[]   = { {659,60,SI,35},{880,60,SI,35},{1319,140,SI,35} };
static const SfxNote FX_SPELL[]  = { {880,40,SI,30},{1175,40,SI,30},{1568,40,SI,30},{2093,90,SI,25} };
#define FX(a) fx_start(a, (int)(sizeof(a) / sizeof(a[0])))

void sfx_select(){ gb::tone_sfx(660.f, 0.25f, 30, gb::TONE_SQUARE); }   // deplacement dans un menu
void sfx_confirm(){ gb::tone_sfx(523.f, 0.35f, 45, gb::TONE_SQUARE); }
void sfx_back(){ gb::tone_sfx(392.f, 0.30f, 45, gb::TONE_TRI); }
void sfx_page(){ gb::tone_sfx(880.f, 0.18f, 18, gb::TONE_SINE); }       // dialogue : ligne suivante
void sfx_coin(){ FX(FX_BUY); }
void sfx_hit(){ gb::tone_sfx(180.f, 0.50f, 60, gb::TONE_SQUARE); }
void sfx_hurt(){ gb::tone_sfx(140.f, 0.55f, 90, gb::TONE_NOISE); }
void sfx_pickup(){ FX(FX_PICKUP); }
void sfx_chest(){ FX(FX_CHEST); }
void sfx_door(){ FX(FX_DOOR); }
void sfx_error(){ FX(FX_ERROR); }
void sfx_craft(){ FX(FX_CRAFT); }
void sfx_open(){ FX(FX_OPEN); }
void sfx_save(){ FX(FX_SAVE); }
void sfx_spell(){ FX(FX_SPELL); }

}} // namespace asteria::audio
