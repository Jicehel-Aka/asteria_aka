// ASTERIA (Gamebuino AKA) - Horloge du jeu (voir daytime.h).
#include "daytime.h"

namespace asteria { namespace daytime {

static int      s_min = 8 * 60;        // minute de la journee
static uint32_t s_acc = 0;             // ms reelles accumulees vers la prochaine minute
static uint32_t s_last = 0;            // date de la derniere image comptee (0 = a reprendre)

void reset(int m){ set_minutes(m); s_acc = 0; s_last = 0; }
void set_minutes(int m){ m %= 1440; if (m < 0) m += 1440; s_min = m; }
int  minutes(){ return s_min; }

void tick(uint32_t now){
  // Un saut > 1 s (scene de combat, menu, chargement) ne compte pas : le temps ne
  // passe qu'en exploration effective.
  if (s_last == 0 || now - s_last > 1000) { s_last = now; return; }
  s_acc += now - s_last; s_last = now;
  while (s_acc >= 1000) { s_acc -= 1000; s_min = (s_min + 1) % 1440; }
}

int ambient(){
  const int NIGHT = 78, DAY = 256;                 // nuit ~30 % de lumiere (+ teinte bleue, voir gb_light.h)
  int m = s_min;
  if (m >= 7 * 60 && m < 18 * 60) return DAY;
  if (m >= 21 * 60 || m < 5 * 60) return NIGHT;
  if (m < 7 * 60)  return NIGHT + (DAY - NIGHT) * (m - 5 * 60) / 120;          // aube 5h -> 7h
  return DAY - (DAY - NIGHT) * (m - 18 * 60) / 180;                            // crepuscule 18h -> 21h
}

bool is_night(){ return s_min >= 20 * 60 || s_min < 6 * 60; }

}}
