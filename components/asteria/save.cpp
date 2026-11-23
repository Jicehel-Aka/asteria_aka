// Sauvegarde / chargement ASTERIA : format texte simple, portable (host/SDL/AKA via write_text/read_text).
#include "save.h"
#include "player.h"
#include "gamestate.h"
#include "scene/world_scene.h"
#include "generated/asteria_maps.h"
#include "platform/gb_port.h"
#include <cstdio>
#include <cstring>
namespace asteria {
bool g_autosave = true;
static const char* SAVE_PATH = "/sdcard/ASTERIA/SAVE.DAT";
static char g_fbuf[64][24];   // stockage persistant des noms de drapeaux charges (flag_set garde le pointeur)

bool game_save(){
  Character& c = player(); WorldScene& w = world_scene();
  static char buf[8192]; int o=0;
  o += snprintf(buf+o, sizeof(buf)-o, "ASTERIA1\n");
  o += snprintf(buf+o, sizeof(buf)-o, "P %d %d %d %d %d %d %d %d %u %d %d %d %d %d %d %d %d %d\n",
     c.IN,c.CO,c.MA,c.AU,c.VIT,c.BL,c.PO,c.weapon,(unsigned)c.spells,c.xp,c.gold,c.level,c.mana,c.manaMax,
     c.attitude[0],c.attitude[1],c.attitude[2],c.attitude[3]);
  o += snprintf(buf+o, sizeof(buf)-o, "W %d %d %d %d\n", w.map, w.px, w.py, w.dir);
  o += snprintf(buf+o, sizeof(buf)-o, "I %d\n", (int)c.inv.n);
  for(int i=0;i<c.inv.n;i++) o += snprintf(buf+o, sizeof(buf)-o, "i %d %d\n", (int)c.inv.slots[i].item, (int)c.inv.slots[i].qty);
  for(int i=0;i<64;i++){ int s=quest_status(i); if(s>0) o += snprintf(buf+o, sizeof(buf)-o, "Q %d %d %d\n", i, s, quest_obj(i)); }
  for(int i=0;i<flag_count();i++){ const char* f=flag_at(i); if(f && f[0] && o < (int)sizeof(buf)-40) o += snprintf(buf+o, sizeof(buf)-o, "F %s\n", f); }
  return gb::write_text(SAVE_PATH, buf);
}

bool game_load(){
  static char buf[8192];
  int n = gb::read_text(SAVE_PATH, buf, sizeof(buf));
  if(n <= 0) return false;
  if(strncmp(buf, "ASTERIA1", 8) != 0) return false;
  state_reset();
  Character& c = player(); c = new_caithness();   // base, puis on ecrase avec la sauvegarde
  WorldScene& w = world_scene();
  int smap=0, spx=1, spy=1, sdir=2, gfn=0; bool haveW=false;
  char* line = strtok(buf, "\n");
  while(line){
    if(line[0]=='P'){ unsigned sp=c.spells;
      sscanf(line,"P %d %d %d %d %d %d %d %d %u %d %d %d %d %d %d %d %d %d",
        &c.IN,&c.CO,&c.MA,&c.AU,&c.VIT,&c.BL,&c.PO,&c.weapon,&sp,&c.xp,&c.gold,&c.level,&c.mana,&c.manaMax,
        &c.attitude[0],&c.attitude[1],&c.attitude[2],&c.attitude[3]);
      c.spells = sp; }
    else if(line[0]=='W'){ sscanf(line,"W %d %d %d %d",&smap,&spx,&spy,&sdir); haveW=true; }
    else if(line[0]=='I'){ c.inv.n=0; }                 // la liste est reconstruite par les lignes 'i'
    else if(line[0]=='i'){ int it,q; if(sscanf(line,"i %d %d",&it,&q)==2 && c.inv.n < Inventory::CAP){
        c.inv.slots[c.inv.n].item=(int16_t)it; c.inv.slots[c.inv.n].qty=(uint8_t)(q<0?0:(q>255?255:q)); c.inv.n++; } }
    else if(line[0]=='Q'){ int i,s,ob; if(sscanf(line,"Q %d %d %d",&i,&s,&ob)==3 && i>=0 && i<64){
        if(s>=1){ quest_start(i); quest_set_obj(i,ob); if(s==2) quest_complete(i); } } }
    else if(line[0]=='F'){ char nm[24]={0}; if(sscanf(line,"F %23s",nm)==1 && nm[0] && gfn<64){
        strncpy(g_fbuf[gfn],nm,23); g_fbuf[gfn][23]=0; flag_set(g_fbuf[gfn]); gfn++; } }
    line = strtok(NULL, "\n");
  }
  if(!haveW) return false;
  if(smap<0 || smap>=data::MAP_COUNT) smap=0;
  w.load_map(smap); w.px=spx; w.py=spy; w.dir=sdir; w.started=true;   // position sauvee (apres load_map qui remet le defaut)
  return true;
}
}
