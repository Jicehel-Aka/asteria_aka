// Port HOST logiciel de gb_port.h : framebuffer 320x240 (BGR565) + export PPM.
// Entrées scriptables via variables d'environnement (pour captures/tests) :
//   ASTERIA_SCRIPT="0,0,5,0,5,..."  (codes: 0=rien 1=haut 2=bas 3=gauche 4=droite 5=A 6=B)
//   ASTERIA_BUDGET="14"             (nombre de frames à rendre)
#include "platform/gb_port.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <string>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define MKDIR(d) mkdir(d,0755)
#endif
#include <vector>
static uint16_t FB[gb::SCREEN_W*gb::SCREEN_H];
static int g_frame=0, g_budget=1; static uint32_t g_ms=0;
static uint32_t code2btn(int c){ switch(c){case 1:return gb::BTN_UP;case 2:return gb::BTN_DOWN;case 3:return gb::BTN_LEFT;case 4:return gb::BTN_RIGHT;case 5:return gb::BTN_A;case 6:return gb::BTN_B;case 7:return gb::BTN_MENU;case 8:return gb::BTN_R1;case 9:return gb::BTN_L1;default:return 0;} }
static int g_script[512]; static int g_scriptN=-1;
static void loadScript(){ g_scriptN=0; const char* e=getenv("ASTERIA_SCRIPT"); if(e){ const char* p=e; while(*p && g_scriptN<512){ g_script[g_scriptN++]=atoi(p); const char* c=strchr(p,','); if(!c)break; p=c+1; } } const char* b=getenv("ASTERIA_BUDGET"); if(b) g_budget=atoi(b); }
namespace gb {
bool running(){ if(g_scriptN<0) loadScript(); return g_frame < g_budget; }
void frame_begin(){}
static void dumpPPM(){ char n[48]; snprintf(n,sizeof(n),"frame_%02d.ppm",g_frame); FILE* fp=fopen(n,"wb"); if(!fp)return;
  fprintf(fp,"P6\n%d %d\n255\n",SCREEN_W,SCREEN_H);
  for(int i=0;i<SCREEN_W*SCREEN_H;i++){ uint16_t c=FB[i]; unsigned b=((c>>11)&0x1F)<<3,g=((c>>5)&0x3F)<<2,r=(c&0x1F)<<3; unsigned char px[3]={(unsigned char)r,(unsigned char)g,(unsigned char)b}; fwrite(px,1,3,fp);} fclose(fp); }
void frame_end(){ dumpPPM(); g_frame++; g_ms+=16; }
void clear(Color c){ for(int i=0;i<SCREEN_W*SCREEN_H;i++) FB[i]=c; }
// Primitives : chemin rapide d'origine si le raster est neutre, sinon passage par
// raster_map() (origine, clip, cible hors ecran — voir gb_raster.h).
static inline void put(int x,int y,Color c){ RasterState& r=raster_state(); if(!raster_map(x,y,SCREEN_W,SCREEN_H)) return; (r.buf?r.buf[y*r.bw+x]:FB[y*SCREEN_W+x])=c; }
void pixel(int x,int y,Color c){ put(x,y,c); }
void fill_rect(int x,int y,int w,int h,Color c){ for(int j=0;j<h;j++)for(int i=0;i<w;i++) put(x+i,y+j,c); }
static std::string mp(const char* p){ if(!p) return ""; std::string s=p,k="/sdcard/";
  if(s.rfind(k,0)!=0 || s.find("..")!=std::string::npos || s.find('\\')!=std::string::npos) return "";  // SEC-05 : anti traversee
  return "./sdcard_files/"+s.substr(k.size()); }
// OPT-AKA-2 : cache du dernier BMP decode. Un combat/ecran reaffiche le meme fond
// a chaque frame ; sans cache on relirait le fichier 60x/s (catastrophique sur SD).
// On garde le dernier chemin + ses pixels RGB565 en RAM et on re-blitte sans relire.
static char     s_bmp_path[160] = {0};
static uint16_t s_bmp_buf[SCREEN_W*SCREEN_H];
static int      s_bmp_w = 0, s_bmp_h = 0;
bool blit_bmp(const char* path){
  if(path && s_bmp_w>0 && strcmp(s_bmp_path,path)==0){                 // cache hit : re-blit depuis la RAM
    for(int y=0;y<s_bmp_h;y++) for(int x=0;x<s_bmp_w;x++) FB[y*SCREEN_W+x]=s_bmp_buf[y*SCREEN_W+x];
    return true;
  }
  FILE* fp=fopen(mp(path).c_str(),"rb"); if(!fp) return false; unsigned char h[54]; if(fread(h,1,54,fp)!=54||h[0]!='B'||h[1]!='M'){fclose(fp);return false;}  // SEC-07 : signature
  int32_t W=(int32_t)(h[18]|(h[19]<<8)|(h[20]<<16)|(h[21]<<24)), H_raw=(int32_t)(h[22]|(h[23]<<8)|(h[24]<<16)|(h[25]<<24));
  int bpp=h[28]|(h[29]<<8); uint32_t off=(uint32_t)(h[10]|(h[11]<<8)|(h[12]<<16)|(h[13]<<24));
  bool flip_y=H_raw>0; int H=flip_y?H_raw:(H_raw==INT32_MIN?0:-H_raw);
  if(bpp!=24 || W<=0 || H<=0 || W>(int)SCREEN_W || H>(int)SCREEN_H || off<54){fclose(fp);return false;}   // SEC-02 : bornes
  if(fseek(fp,(long)off,SEEK_SET)!=0){fclose(fp);return false;}
  int row=((W*3+3)/4)*4; std::vector<unsigned char> b(row);
  for(int yy=0;yy<H;yy++){ if(fread(b.data(),1,row,fp)!=(size_t)row){ fclose(fp); return false; } int dy=flip_y?(H-1-yy):yy;
    for(int xx=0;xx<W;xx++) s_bmp_buf[dy*SCREEN_W+xx]=rgb(b[xx*3+2],b[xx*3+1],b[xx*3]); }   // decode -> cache
  fclose(fp);
  s_bmp_w=W; s_bmp_h=H; strncpy(s_bmp_path,path,sizeof(s_bmp_path)-1); s_bmp_path[sizeof(s_bmp_path)-1]=0;
  for(int y=0;y<H;y++) for(int x=0;x<W;x++) FB[y*SCREEN_W+x]=s_bmp_buf[y*SCREEN_W+x];          // cache -> ecran
  return true; }
uint32_t buttons(){ return 0; }
static int g_inject=-1;   // autopilote : injection d'entree frame par frame
uint32_t buttons_pressed(){ if(g_inject>=0) return code2btn(g_inject); if(g_scriptN<0) loadScript(); return (g_frame<g_scriptN)?code2btn(g_script[g_frame]):0; }
uint32_t millis(){ return g_ms; }
bool file_exists(const char* path){ FILE* f=fopen(mp(path).c_str(),"rb"); if(f){fclose(f);return true;} return false; }
void return_to_loader(){}
void log(const char* m){ printf("[host] %s\n",m); }
}
void host_set_budget(int n){ g_budget=n; }
void host_inject(int code){ gb::g_inject=code; }   // -1 = aucune entree
void host_tick(){ g_ms+=16; }                      // avance l'horloge (logique temporelle)

namespace gb {
void draw_image(const uint16_t* px, uint16_t w, uint16_t h, int x, int y){
    for(int j=0;j<h;++j) for(int i=0;i<w;++i) put(x+i,y+j,px[j*w+i]);
}
void draw_image_key(const uint16_t* px, uint16_t w, uint16_t h, int x, int y, uint16_t key){
    for(int j=0;j<h;++j) for(int i=0;i<w;++i){ uint16_t c=px[j*w+i]; if(c!=key) put(x+i,y+j,c); }   // SEC-07 : put() clippe
}
void blit_region(const uint16_t* src, int src_w, int src_h, int sx, int sy, int w, int h, int dx, int dy){
    for(int j=0;j<h;++j){ int yy=sy+j; if((unsigned)yy>=(unsigned)src_h) continue;
      for(int i=0;i<w;++i){ int xx=sx+i; if((unsigned)xx<(unsigned)src_w) put(dx+i,dy+j,src[yy*src_w+xx]); } }
}
void light_screen(int y0, int y1, int ambient, const Light* lights, int n){
    if(y0<0)y0=0; if(y1>SCREEN_H)y1=SCREEN_H;
    for(int y=y0;y<y1;++y) light_row(&FB[y*SCREEN_W],SCREEN_W,y,ambient,lights,n);
}
void* alloc_big(unsigned bytes){ return malloc(bytes); }
void  free_big(void* p){ free(p); }
}

namespace gb {
// SEC-07 : cree les dossiers parents (ex. /sdcard/ASTERIA/) avant d'ecrire.
static void mkdirs_for(const std::string& file){
  for(size_t i=1;i<file.size();++i) if(file[i]=='/'){ std::string d=file.substr(0,i); MKDIR(d.c_str()); }
}
bool write_text(const char* path, const char* content){
    std::string s=mp(path); if(s.empty()||!content) return false;   // SEC-05
    mkdirs_for(s);
    FILE* f=fopen(s.c_str(),"w"); if(!f) return false; fputs(content,f); fclose(f); return true;
}
}

namespace gb {
int read_bin(const char* path, void* out, int maxlen){
    std::string s=mp(path); if(s.empty()||!out||maxlen<=0) return -1;
    FILE* fp=fopen(s.c_str(),"rb"); if(!fp) return -1;               // "rb" : pas de conversion de fins de ligne (Windows)
    int n=(int)fread(out,1,(size_t)maxlen,fp); fclose(fp); return n;
}
int read_text(const char* path, char* out, int maxlen){
    std::string s=mp(path); if(s.empty()||maxlen<=0) return -1;   // SEC-05
    FILE* f=fopen(s.c_str(),"r"); if(!f) return -1; int n=(int)fread(out,1,maxlen-1,f); fclose(f); if(n<0)n=0; out[n]=0; return n;
}
}

namespace gb {
// Audio silencieux côté host (captures/tests reproductibles, pas de son).
void audio_init(){}
void tone_music(float, float, uint16_t, uint8_t){}
void tone_sfx(float, float, uint16_t, uint8_t){}
void audio_stop(){}
bool music_play(const void*){ return false; }   // host sans son
void music_stop(){}
void music_volume(float){}
}
