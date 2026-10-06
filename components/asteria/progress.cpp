#include "progress.h"
#include "gamestate.h"
#include "player.h"
#include "audio/jingle.h"
#include "generated/asteria_quest.h"
namespace asteria {

bool gain_xp(int xp){
  if(xp<=0) return false;
  Character& p=player();
  int l0=p.level;
  p.xp+=xp;
  // Même règle que le combat : on retire niveau*100 à chaque palier, +5 VIT.
  while(p.xp >= p.level*100){ p.xp -= p.level*100; p.level++; p.VIT += 5; }
  return p.level>l0;
}

void complete_quest_reward(int qi){
  if(qi<0 || qi>=quest::QUEST_COUNT) return;
  if(quest_status(qi)==2) return;                 // déjà terminée : pas de double récompense
  quest_complete(qi);
  bool leveled = gain_xp(quest::QUEST[qi].xp);
  audio::play(leveled ? audio::TUNE_LEVELUP : audio::TUNE_VICTORY);
}

// Découvertes : tableau local (les flags gamestate stockent le POINTEUR de la
// chaîne, donc on n'y met pas de clé construite dynamiquement).
static bool g_disc[64];
void discover_zone(int loc){
  if(loc<0 || loc>=64 || g_disc[loc]) return;
  g_disc[loc]=true;
  gain_xp(50);
}
void progress_reset(){ for(int i=0;i<64;i++) g_disc[i]=false; }

}
