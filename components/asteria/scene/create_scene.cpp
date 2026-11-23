// ASTERIA (Gamebuino AKA) - Ecran de creation : heroine pretiree Caithness, ou voyageur cree (nom + sexe, puis repartition des points), fond illustration HD.
#include "scene/create_scene.h"
#include "i18n.h"
#include "ui/text.h"
#include "platform/gb_port.h"
#include "player.h"
#include "scene/world_scene.h"
#include <cstdio>
#include <cstring>
#include "content_i18n.h"
namespace asteria {
static const int BASE=8, MINV=8, MAXV=30, POOL0=18;
CreateScene& create_scene(){ static CreateScene s; return s; }
void CreateScene::enter(){ new_game_reset(); step=0; selChoice=0; cur=0; st[0]=st[1]=st[2]=st[3]=BASE; pool=POOL0;
  strcpy(nm,"Aren        "); ncur=0; editing=false; female=false; idSel=0; }
// Jeu de caracteres du nom (haut/bas fait defiler) ; ' ' = case vide.
static const char NAMESET[]=" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'";
static int nameset_idx(char ch){ for(int i=0;NAMESET[i];i++) if(NAMESET[i]==ch) return i; return 0; }
// Nom saisi sans les espaces de fin (et "Aren" si vide).
static void trimmed(const char* in,char* out,int cap){ int n=(int)strlen(in); while(n>0&&in[n-1]==' ') n--; int s=0; while(s<n&&in[s]==' ') s++;
  int k=0; for(int i=s;i<n&&k<cap-1;i++) out[k++]=in[i]; out[k]=0; if(!out[0]) strncpy(out,"Aren",cap); }
static void buildCustom(const int* st,const char* nm,bool female){ Character c=new_caithness(); c.IN=st[0];c.CO=st[1];c.MA=st[2];c.AU=st[3];
  c.manaMax=6+c.MA/2; c.mana=c.manaMax;                    // le pouvoir suit la MA choisie
  trimmed(nm,c.name,sizeof(c.name)); c.female=female; player()=c; }
void CreateScene::update(SceneManager& m){
  uint32_t p=gb::buttons_pressed();
  if(step==0){
    if(p&gb::BTN_DOWN) selChoice=1;
    if(p&gb::BTN_UP)   selChoice=0;
    if(p&gb::BTN_B)    m.set(SceneId::TITLE);
    if(p&gb::BTN_A){ if(selChoice==0){ player()=new_caithness(); step=2; } else { step=3; idSel=0; editing=false; } }
  } else if(step==1){
    if(p&gb::BTN_DOWN) cur=(cur+1)%6;
    if(p&gb::BTN_UP)   cur=(cur+5)%6;
    if(p&gb::BTN_A){ if(cur<4){ if(pool>0&&st[cur]<MAXV){st[cur]++;pool--;} } else if(cur==4){ buildCustom(st,nm,female); step=2; } else { step=3; } }
    if(p&gb::BTN_B){ if(cur<4 && st[cur]>MINV){ st[cur]--; pool++; } }
  } else if(step==3){                                   // DELTA-02 : identite du voyageur
    if(editing){
      int L=(int)strlen(nm);
      if(p&gb::BTN_UP)   { int i=nameset_idx(nm[ncur]); nm[ncur]=NAMESET[(i+1)%(int)(sizeof(NAMESET)-1)]; }
      if(p&gb::BTN_DOWN) { int i=nameset_idx(nm[ncur]); int n=(int)(sizeof(NAMESET)-1); nm[ncur]=NAMESET[(i+n-1)%n]; }
      if(p&gb::BTN_RIGHT && ncur<L-1) ncur++;
      if(p&gb::BTN_LEFT  && ncur>0)   ncur--;
      if(p&(gb::BTN_A|gb::BTN_B)) editing=false;
    } else {
      if(p&gb::BTN_DOWN) idSel=(idSel+1)%4;
      if(p&gb::BTN_UP)   idSel=(idSel+3)%4;
      if(idSel==1 && (p&(gb::BTN_LEFT|gb::BTN_RIGHT))) female=!female;
      if(p&gb::BTN_A){ if(idSel==0){ editing=true; ncur=0; } else if(idSel==1) female=!female; else if(idSel==2){ step=1; cur=0; } else step=0; }
      if(p&gb::BTN_B) step=0;
    }
  } else {
    if(p&gb::BTN_A) m.set(SceneId::WORLD);
    if(p&gb::BTN_B) step=(selChoice==1)?1:0;
  }
}
void CreateScene::render(){
  if(!gb::blit_bmp("/sdcard/ASTERIA/CAITH.BMP")) gb::clear(gb::rgb(26,20,14));   // illustration HD de Caithness en fond
  if(step==0){
    ui::text_big(64,14,i18n::tr(i18n::CR_TITLE),gb::rgb(230,210,140));
    const char* o[2]={i18n::tr(i18n::CR_PRESET),i18n::tr(i18n::CR_CUSTOM)};
    for(int i=0;i<2;i++){ int y=72+i*20;
      if(i==selChoice){ gb::fill_rect(18,y-3,286,17,gb::rgb(60,45,20)); ui::gold(22,y,">",true); }
      ui::text(38,y,o[i],gb::rgb(236,226,192)); }
    ui::text(18,214,i18n::tr(i18n::CR_VALID_B),gb::rgb(180,170,140));
  } else if(step==1){
    ui::text_big(56,10,i18n::tr(i18n::CR_POINTS_T),gb::rgb(230,210,140));
    char buf[40]; snprintf(buf,sizeof(buf),"%s %d",i18n::tr(i18n::CR_REMAIN),pool); ui::text(92,34,buf,gb::rgb(224,214,182));
    const char* rows[6]={"IN","CO","MA","AU",i18n::tr(i18n::CR_VALIDATE),i18n::tr(i18n::CR_BACK)};
    for(int i=0;i<6;i++){ int y=58+i*20;
      if(i==cur){ gb::fill_rect(40,y-3,244,17,gb::rgb(60,45,20)); ui::gold(44,y,">",true); }
      if(i<4){ char l[24]; snprintf(l,sizeof(l),"%s : %d",rows[i],st[i]); ui::text(62,y,l,gb::rgb(236,226,192)); }
      else ui::text(62,y,rows[i],gb::rgb(236,226,192)); }
    ui::text(8,216,i18n::tr(i18n::CR_PM_HINT),gb::rgb(180,170,140));
  } else if(step==3){
    ui::text_big(80,14,trc("Ton voyageur"),gb::rgb(230,210,140));
    const int Y0=66;
    for(int i=0;i<4;i++){ int y=Y0+i*22;
      if(i==idSel){ gb::fill_rect(40,y-3,244,17,gb::rgb(60,45,20)); ui::gold(44,y,">",true); } }
    char l[40]; snprintf(l,sizeof(l),"%s : ",trc("Nom")); ui::text(62,Y0,l,gb::rgb(236,226,192));
    int nx=62+ui::text_w(l);
    if(!editing){ char t[13]; trimmed(nm,t,sizeof(t)); ui::text(nx,Y0,t,gb::rgb(250,240,200)); }
    else for(int k=0;k<12;k++){ char one[2]={nm[k],0}; int cx=nx+k*9;     // edition : une case par lettre
      if(editing && k==ncur) gb::fill_rect(cx-1,Y0+13,9,2,gb::rgb(245,215,95));
      if(nm[k]!=' ') ui::text(cx,Y0,one,gb::rgb(250,240,200)); else if(editing) ui::text(cx,Y0,"_",gb::rgb(120,110,90)); }
    snprintf(l,sizeof(l),"%s : < %s >",trc("Sexe"),trc(female?"Femme":"Homme")); ui::text(62,Y0+22,l,gb::rgb(236,226,192));
    ui::text(62,Y0+44,trc("Continuer"),gb::rgb(236,226,192));
    ui::text(62,Y0+66,trc("Retour"),gb::rgb(236,226,192));
    ui::text(8,216,trc(editing?"Haut/Bas : lettre   A/B : fini":"A : modifier   B : retour"),gb::rgb(180,170,140));
  } else {
    Character& c=player();
    ui::text_big(96,10,i18n::tr(i18n::CR_SHEET),gb::rgb(230,210,140));
    ui::gold(34,30,c.name,true);
    char b[40];
    snprintf(b,sizeof(b),"IN %d    CO %d",c.IN,c.CO); ui::text(34,44,b,gb::rgb(236,226,192));
    snprintf(b,sizeof(b),"MA %d    AU %d",c.MA,c.AU); ui::text(34,64,b,gb::rgb(236,226,192));
    snprintf(b,sizeof(b),"VIT %d   BL %d",c.VIT,c.BL); ui::text(34,84,b,gb::rgb(236,226,192));
    snprintf(b,sizeof(b),"Pouvoir %d",c.PO);          ui::text(34,104,b,gb::rgb(236,226,192));
    ui::text(34,132,i18n::tr(i18n::CR_WEAPON),gb::rgb(220,210,180));
    ui::text(34,152,i18n::tr(i18n::CR_SPELLS1),gb::rgb(210,200,170));
    ui::text(34,166,i18n::tr(i18n::CR_SPELLS2),gb::rgb(210,200,170));
    ui::gold(44,208,i18n::tr(i18n::CR_START),true);
  }
}
}
