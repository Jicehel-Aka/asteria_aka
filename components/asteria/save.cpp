// Sauvegarde / chargement ASTERIA : 3 emplacements portables, tampon temporaire alloc_big (PSRAM/heap).
#include "save.h"
#include "daytime.h"
#include "player.h"
#include "gamestate.h"
#include "scene/world_scene.h"
#include "generated/asteria_maps.h"
#include "platform/gb_port.h"
#include <cstdio>
#include <cstring>

namespace asteria {

bool g_autosave = true;
static int g_slot = 1;
static char g_fbuf[64][24];   // stockage persistant des noms de drapeaux charges (flag_set garde le pointeur)

// Decoupe ligne par ligne EN PLACE (remplace strtok_r, absent de MinGW en mode
// strict -std=c++17). Renvoie la ligne courante et avance *cur ; nullptr a la fin.
static char* next_line(char** cur){
  char* s = *cur;
  while (s && *s == '\n') ++s;            // ignore les lignes vides
  if (!s || !*s) { *cur = s; return nullptr; }
  char* e = strchr(s, '\n');
  if (e) { *e = 0; *cur = e + 1; } else *cur = s + strlen(s);
  return s;
}

// SAVEn.DAT (n = 1..3) ; un numero invalide retombe sur l'emplacement 1.
static const char* slot_path(int slot, char* out, int cap){
  int s = (slot < 1 || slot > 3) ? 1 : slot;
  snprintf(out, cap, "/sdcard/ASTERIA/SAVE%d.DAT", s);
  return out;
}

static const char* read_slot_path(int slot, char* out, int cap){
  slot_path(slot, out, cap);
  if (gb::file_exists(out)) return out;
  if (slot == 1 && gb::file_exists("/sdcard/ASTERIA/SAVE.DAT")) {
    snprintf(out, cap, "/sdcard/ASTERIA/SAVE.DAT");
  }
  return out;
}

int active_save_slot(){ return g_slot; }
void set_active_save_slot(int slot){ if(slot>=1 && slot<=3) g_slot = slot; }

bool save_slot_summary(int slot, SaveSummary& out){
  out = SaveSummary{};
  char path[64]; read_slot_path(slot, path, sizeof(path));
  char head[512];
  int n = gb::read_text(path, head, sizeof(head));
  if (n <= 0 || strncmp(head, "ASTERIA1", 8) != 0) return false;
  out.exists = true;
  out.level = 1;
  strcpy(out.name, "Caithness");
  char* cur = head;
  char* line = next_line(&cur);
  while (line) {
    if (line[0] == 'P') {
      int dummy, xp=0, gold=0, lvl=1;
      unsigned sp=0;
      if (sscanf(line, "P %d %d %d %d %d %d %d %d %u %d %d %d",
                 &dummy,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,&sp,&xp,&gold,&lvl) >= 12) {
        out.gold = gold;
        out.level = lvl;
      }
    } else if (line[0] == 'N') {
      int f=1, off=0;
      if (sscanf(line, "N %d %n", &f, &off) >= 1 && off > 0 && line[off]) {
        strncpy(out.name, line + off, sizeof(out.name) - 1);
        out.name[sizeof(out.name) - 1] = 0;
      }
    } else if (line[0] == 'W') {
      int mp=0;
      if (sscanf(line, "W %d", &mp) == 1) out.map = mp;
    }
    line = next_line(&cur);
  }
  return true;
}

bool game_save_slot(int slot){
  if (slot >= 1 && slot <= 3) g_slot = slot;
  Character& c = player(); WorldScene& w = world_scene();
  const int CAP = 8192;
  char* buf = (char*)gb::alloc_big(CAP);
  if (!buf) return false;
  int o = 0;
  auto app = [&](int written){
    if (written > 0) o += written;
    if (o >= CAP) o = CAP - 1;
  };
  app(snprintf(buf+o, CAP-o, "ASTERIA1\n"));
  app(snprintf(buf+o, CAP-o, "P %d %d %d %d %d %d %d %d %u %d %d %d %d %d %d %d %d %d\n",
      c.IN,c.CO,c.MA,c.AU,c.VIT,c.BL,c.PO,c.weapon,(unsigned)c.spells,c.xp,c.gold,c.level,c.mana,c.manaMax,
      c.attitude[0],c.attitude[1],c.attitude[2],c.attitude[3]));
  app(snprintf(buf+o, CAP-o, "L %d\n", c.torchFuel));
  app(snprintf(buf+o, CAP-o, "N %d %s\n", c.female?1:0, c.name));
  app(snprintf(buf+o, CAP-o, "W %d %d %d %d\n", w.map, w.px, w.py, w.dir));
  app(snprintf(buf+o, CAP-o, "T %d\n", daytime::minutes()));   // heure du jeu (absente des vieilles sauvegardes -> 8h)
  app(snprintf(buf+o, CAP-o, "I %d\n", (int)c.inv.n));
  for (int i=0; i<c.inv.n && o < CAP-32; i++)
    app(snprintf(buf+o, CAP-o, "i %d %d\n", (int)c.inv.slots[i].item, (int)c.inv.slots[i].qty));
  for (int i=0; i<64 && o < CAP-32; i++) {
    int s = quest_status(i);
    if (s > 0) app(snprintf(buf+o, CAP-o, "Q %d %d %d\n", i, s, quest_obj(i)));
  }
  for (int i=0; i<flag_count() && o < CAP-40; i++) {
    const char* f = flag_at(i);
    if (f && f[0]) app(snprintf(buf+o, CAP-o, "F %s\n", f));
  }
  buf[CAP-1] = 0;
  char path[64]; slot_path(g_slot, path, sizeof(path));
  bool ok = gb::write_text(path, buf);
  gb::free_big(buf);
  return ok;
}

bool game_load_slot(int slot){
  if (slot >= 1 && slot <= 3) g_slot = slot;
  const int CAP = 8192;
  char* buf = (char*)gb::alloc_big(CAP);
  if (!buf) return false;
  char path[64]; read_slot_path(g_slot, path, sizeof(path));
  int n = gb::read_text(path, buf, CAP);
  if (n <= 0 || strncmp(buf, "ASTERIA1", 8) != 0) { gb::free_big(buf); return false; }

  state_reset();
  Character& c = player(); c = new_caithness();   // base, puis on ecrase avec la sauvegarde
  WorldScene& w = world_scene();
  int smap=0, spx=1, spy=1, sdir=2, gfn=0, tmin=8*60; bool haveW=false;
  char* cur = buf;
  char* line = next_line(&cur);
  while (line) {
    if (line[0] == 'P') {
      unsigned sp = c.spells;
      sscanf(line, "P %d %d %d %d %d %d %d %d %u %d %d %d %d %d %d %d %d %d",
        &c.IN,&c.CO,&c.MA,&c.AU,&c.VIT,&c.BL,&c.PO,&c.weapon,&sp,&c.xp,&c.gold,&c.level,&c.mana,&c.manaMax,
        &c.attitude[0],&c.attitude[1],&c.attitude[2],&c.attitude[3]);
      c.spells = sp;
    } else if (line[0] == 'L') {
      int tf = 100;
      if (sscanf(line, "L %d", &tf) == 1) {
        c.torchFuel = (tf < 0) ? 0 : (tf > 100 ? 100 : tf);
      }
    } else if (line[0] == 'N') {
      int f=1, off=0;
      if (sscanf(line, "N %d %n", &f, &off) >= 1 && off > 0) {
        c.female = (f != 0);
        strncpy(c.name, line+off, sizeof(c.name)-1);
        c.name[sizeof(c.name)-1] = 0;
        if (!c.name[0]) strcpy(c.name, "Caithness");
      }
    } else if (line[0] == 'T') {
      int t = 8 * 60; if (sscanf(line, "T %d", &t) == 1) tmin = t;
    } else if (line[0] == 'W') {
      sscanf(line, "W %d %d %d %d", &smap, &spx, &spy, &sdir);
      haveW = true;
    } else if (line[0] == 'I') {
      c.inv.n = 0;
    } else if (line[0] == 'i') {
      int it, q;
      if (sscanf(line, "i %d %d", &it, &q) == 2 && it >= 0 && q > 0) {
        c.inv.add(it, q);
      }
    } else if (line[0] == 'Q') {
      int i, s, ob;
      if (sscanf(line, "Q %d %d %d", &i, &s, &ob) == 3 && i >= 0 && i < 64) {
        if (s >= 1) { quest_start(i); quest_set_obj(i, ob); if (s == 2) quest_complete(i); }
      }
    } else if (line[0] == 'F') {
      char nm[24] = {0};
      if (sscanf(line, "F %23s", nm) == 1 && nm[0] && gfn < 64) {
        strncpy(g_fbuf[gfn], nm, 23); g_fbuf[gfn][23] = 0;
        flag_set(g_fbuf[gfn]); gfn++;
      }
    }
    line = next_line(&cur);
  }
  gb::free_big(buf);
  if (!haveW) return false;
  if (smap < 0 || smap >= data::MAP_COUNT) smap = 0;
  w.load_map(smap); w.px = spx; w.py = spy; w.dir = sdir; w.started = true;   // position sauvee (apres load_map qui remet le defaut)
  daytime::reset(tmin);
  return true;
}

bool game_save(){ return game_save_slot(g_slot); }
bool game_load(){ return game_load_slot(g_slot); }

}
