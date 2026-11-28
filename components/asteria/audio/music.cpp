// ASTERIA (Gamebuino AKA) - Musique d'ambiance (voir music.h).
#include "audio/music.h"
#include "platform/gb_port.h"
#include "daytime.h"
#include "audio/jingle.h"
#include "scene/world_scene.h"
#include "generated/asteria_maps.h"

namespace asteria { namespace music {

static const char* const FILES[THEME_COUNT] = {
  nullptr,
  "/sdcard/ASTERIA/music/TITRE.PMF",   "/sdcard/ASTERIA/music/VALBOIS.PMF", "/sdcard/ASTERIA/music/NUIT.PMF",
  "/sdcard/ASTERIA/music/FORET.PMF",   "/sdcard/ASTERIA/music/MINE.PMF",    "/sdcard/ASTERIA/music/ROUTE.PMF",
  "/sdcard/ASTERIA/music/COMBAT.PMF",  "/sdcard/ASTERIA/music/TAVERNE.PMF" };

static const int BUF_CAP = 96 * 1024;   // plus gros morceau actuel : ~20 Ko
static uint8_t*  s_buf[2] = { nullptr, nullptr };
static int       s_cur = 0;             // tampon du morceau en cours
static Theme     s_theme = NONE;
static bool      s_hold = false;        // jingle en cours : pas de musique
static SceneId   s_holdScene = SceneId::TITLE;   // scene ou le jingle a commence
static SceneId   s_lastScene = SceneId::QUIT;
static bool      s_failed[THEME_COUNT] = {};   // fichier absent : ne pas relire la SD a chaque image

const char* file_of(Theme t){ return (t > NONE && t < THEME_COUNT) ? FILES[t] : nullptr; }
Theme current(){ return s_theme; }

void stop(){ gb::music_stop(); s_theme = NONE; }

void play(Theme t){
  if (t == s_theme) return;
  if (t == NONE || s_failed[t]) { stop(); s_theme = t; return; }
  // Tampon LIBRE (pas celui que le lecteur est peut-etre en train de lire).
  int nb = s_theme == NONE ? s_cur : 1 - s_cur;
  if (!s_buf[nb]) s_buf[nb] = (uint8_t*)gb::alloc_big(BUF_CAP);
  if (!s_buf[nb]) { stop(); return; }
  int n = gb::read_bin(FILES[t], s_buf[nb], BUF_CAP);
  if (n < 64 || n >= BUF_CAP) { s_failed[t] = true; stop(); s_theme = t; gb::log("MUSIQUE: fichier absent ou trop gros"); return; }
  if (gb::music_play(s_buf[nb])) { s_cur = nb; s_theme = t; }
  else { s_failed[t] = true; stop(); s_theme = t; }
}

// Theme d'une zone d'exploration (lieu + heure).
static Theme world_theme(){
  const WorldScene& w = world_scene();
  int loc = data::MAPS[w.map].location;
  bool night = daytime::is_night();
  switch (loc) {
    case 1:            return TAVERN;                       // taverne de Valbois
    case 8: case 23:   return FOREST;                       // foret de Brume, vallee des Pierres
    case 9: case 21:   return MINE;                         // vieille mine, antre malefique
    case 20: case 22:  return night ? NIGHT : ROAD;         // route royale, grande plaine
    default:           return night ? NIGHT : VILLAGE;      // Valbois, Grand-Castel, maisons
  }
}

void hold_for_jingle(){ s_hold = true; s_holdScene = s_lastScene; gb::music_stop(); s_theme = NONE; }

void update(SceneId scene){
  if (scene != s_lastScene) s_lastScene = scene;
  // Fin de la coupure pour jingle : des que le jingle est fini en exploration ; en
  // combat / game over, seulement en quittant la scene (sinon la musique de combat
  // reprendrait par-dessus l'ecran de victoire).
  if (s_hold) {
    bool sticky = s_holdScene == SceneId::COMBAT || s_holdScene == SceneId::GAMEOVER;
    if (sticky ? scene != s_holdScene : !audio::playing()) s_hold = false;
    else return;
  }
  Theme want = s_theme;
  switch (scene) {
    case SceneId::TITLE: case SceneId::CREATE: case SceneId::RULES: want = TITLE; break;
    case SceneId::WORLD:    want = world_theme(); break;
    case SceneId::DUNGEON:  want = MINE; break;
    case SceneId::COMBAT:   want = COMBAT; break;
    case SceneId::INN:      want = TAVERN; break;
    case SceneId::GAMEOVER: want = NONE; break;
    default: break;                                          // boutique, forge, des, editeur : on garde
  }
  if (want != s_theme) play(want);
}

}}
