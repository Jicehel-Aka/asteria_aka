#include "audio/jingle.h"
#include "platform/gb_port.h"

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
  switch(t){
    case TUNE_DEATH:   start(DEATH,   (int)(sizeof(DEATH)/sizeof(Note)),   gb::TONE_TRI);    break;
    case TUNE_VICTORY: start(VICTORY, (int)(sizeof(VICTORY)/sizeof(Note)), gb::TONE_SQUARE); break;
    case TUNE_LEVELUP: start(LEVELUP, (int)(sizeof(LEVELUP)/sizeof(Note)), gb::TONE_SQUARE); break;
    case TUNE_TITLE:   start(TITLE,   (int)(sizeof(TITLE)/sizeof(Note)),   gb::TONE_TRI);    break;
    default: g_active=false; break;
  }
}

void update(){
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
void sfx_select(){ gb::tone_sfx(660.f, 0.35f, 40, gb::TONE_SQUARE); }
void sfx_confirm(){ gb::tone_sfx(523.f, 0.45f, 55, gb::TONE_SQUARE); }
void sfx_coin(){ gb::tone_sfx(988.f, 0.40f, 45, gb::TONE_SQUARE); }
void sfx_hit(){ gb::tone_sfx(180.f, 0.50f, 60, gb::TONE_SQUARE); }
void sfx_hurt(){ gb::tone_sfx(140.f, 0.55f, 90, gb::TONE_NOISE); }

}} // namespace asteria::audio
