// Port HOST logiciel de gb_port.h : framebuffer 320x240 (BGR565) + export PPM.
// Entrées scriptables via variables d'environnement (pour captures/tests) :
//   ASTERIA_SCRIPT="0,0,5,0,5,..."  (codes: 0=rien 1=haut 2=bas 3=gauche 4=droite 5=A 6=B)
//   ASTERIA_BUDGET="14"             (nombre de frames à rendre)
#include "platform/gb_port.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
static uint16_t FB[gb::SCREEN_W*gb::SCREEN_H];
static int g_frame=0, g_budget=1; static uint32_t g_ms=0;
static uint32_t code2btn(int c){ switch(c){case 1:return gb::BTN_UP;case 2:return gb::BTN_DOWN;case 3:return gb::BTN_LEFT;case 4:return gb::BTN_RIGHT;case 5:return gb::BTN_A;case 6:return gb::BTN_B;case 7:return gb::BTN_MENU;case 8:return gb::BTN_RUN;default:return 0;} }
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
void pixel(int x,int y,Color c){ if((unsigned)x<(unsigned)SCREEN_W&&(unsigned)y<(unsigned)SCREEN_H) FB[y*SCREEN_W+x]=c; }
void fill_rect(int x,int y,int w,int h,Color c){ for(int j=0;j<h;j++)for(int i=0;i<w;i++) pixel(x+i,y+j,c); }
static std::string mp(const char* p){ std::string s=p,k="/sdcard/"; if(s.rfind(k,0)==0) s="./sdcard_files/"+s.substr(k.size()); return s; }
bool blit_bmp(const char* path){ FILE* fp=fopen(mp(path).c_str(),"rb"); if(!fp) return false; unsigned char h[54]; if(fread(h,1,54,fp)!=54){fclose(fp);return false;}
  int W=h[18]|h[19]<<8|h[20]<<16|h[21]<<24,H=h[22]|h[23]<<8|h[24]<<16|h[25]<<24,bpp=h[28]|h[29]<<8,off=h[10]|h[11]<<8|h[12]<<16|h[13]<<24;
  if(bpp!=24){fclose(fp);return false;} fseek(fp,off,SEEK_SET); int row=((W*3+3)/4)*4; std::vector<unsigned char> b(row);
  for(int yy=0;yy<H;yy++){ if(fread(b.data(),1,row,fp)!=(size_t)row)break; int dy=H-1-yy; for(int xx=0;xx<W;xx++){ if(dy<SCREEN_H&&xx<SCREEN_W) FB[dy*SCREEN_W+xx]=rgb(b[xx*3+2],b[xx*3+1],b[xx*3]);} } fclose(fp); return true; }
uint32_t buttons(){ return 0; }
uint32_t buttons_pressed(){ if(g_scriptN<0) loadScript(); return (g_frame<g_scriptN)?code2btn(g_script[g_frame]):0; }
uint32_t millis(){ return g_ms; }
bool file_exists(const char* path){ FILE* f=fopen(mp(path).c_str(),"rb"); if(f){fclose(f);return true;} return false; }
void return_to_loader(){}
void log(const char* m){ printf("[host] %s\n",m); }
}
void host_set_budget(int n){ g_budget=n; }

namespace gb {
void draw_image(const uint16_t* px, uint16_t w, uint16_t h, int x, int y){
    for(uint16_t j=0;j<h;++j) for(uint16_t i=0;i<w;++i) pixel(x+i,y+j,px[j*w+i]);
}
void draw_image_key(const uint16_t* px, uint16_t w, uint16_t h, int x, int y, uint16_t key){
    for(uint16_t j=0;j<h;++j) for(uint16_t i=0;i<w;++i){ uint16_t c=px[j*w+i]; if(c!=key) pixel(x+i,y+j,c); }
}
}

namespace gb {
bool write_text(const char* path, const char* content){
    std::string s=path, k="/sdcard/"; if(s.rfind(k,0)==0) s="./sdcard_files/"+s.substr(k.size());
    FILE* f=fopen(s.c_str(),"w"); if(!f) return false; fputs(content,f); fclose(f); return true;
}
}

namespace gb {
int read_text(const char* path, char* out, int maxlen){
    std::string s=path, k="/sdcard/"; if(s.rfind(k,0)==0) s="./sdcard_files/"+s.substr(k.size());
    FILE* f=fopen(s.c_str(),"r"); if(!f) return -1; int n=(int)fread(out,1,maxlen-1,f); fclose(f); if(n<0)n=0; out[n]=0; return n;
}
}
