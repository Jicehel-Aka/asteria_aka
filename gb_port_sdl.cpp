// Port SDL2 de gb_port.h : fenêtre 320x240 (x2), clavier, blit BMP depuis ./sdcard_files.
// Build: g++ -std=c++17 -I<comp_asteria> <sources_asteria> gb_port_sdl.cpp main_sdl.cpp $(sdl2-config --cflags --libs) -o asteria_pc
#include "platform/gb_port.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
static uint16_t FB[gb::SCREEN_W*gb::SCREEN_H];
static SDL_Window* win=nullptr; static SDL_Renderer* ren=nullptr; static SDL_Texture* tex=nullptr;
static bool g_quit=false; static uint32_t g_held=0, g_pressed=0;
static const int SCALE=2;
static void pump(){
  g_pressed=0; SDL_Event e;
  while(SDL_PollEvent(&e)){
    if(e.type==SDL_QUIT) g_quit=true;
    if(e.type==SDL_KEYDOWN||e.type==SDL_KEYUP){
      bool d=(e.type==SDL_KEYDOWN); uint32_t b=0;
      switch(e.key.keysym.sym){
        case SDLK_UP: case SDLK_z: b=gb::BTN_UP; break;
        case SDLK_DOWN: case SDLK_s: b=gb::BTN_DOWN; break;
        case SDLK_LEFT: case SDLK_q: b=gb::BTN_LEFT; break;
        case SDLK_RIGHT: case SDLK_d: b=gb::BTN_RIGHT; break;
        case SDLK_x: case SDLK_RETURN: b=gb::BTN_A; break;
        case SDLK_c: case SDLK_BACKSPACE: b=gb::BTN_B; break;
        case SDLK_RETURN2: b=gb::BTN_MENU; break;
        case SDLK_ESCAPE: g_quit=true; break;
      }
      if(b){ if(d){ if(!(g_held&b)) g_pressed|=b; g_held|=b; } else g_held&=~b; }
    }
  }
}
namespace gb {
bool running(){ return !g_quit; }
void frame_begin(){
  if(!win){ SDL_Init(SDL_INIT_VIDEO);
    win=SDL_CreateWindow("ASTERIA (PC)",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_W*SCALE,SCREEN_H*SCALE,0);
    ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    tex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_RGB565,SDL_TEXTUREACCESS_STREAMING,SCREEN_W,SCREEN_H);
  }
  pump();
}
void frame_end(){
  // FB est en BGR565 (convention AKA) ; on convertit en RGB565 pour SDL
  static uint16_t conv[SCREEN_W*SCREEN_H];
  for(int i=0;i<SCREEN_W*SCREEN_H;i++){ uint16_t c=FB[i];
    unsigned b=(c>>11)&0x1F,g=(c>>5)&0x3F,r=c&0x1F; conv[i]=(r<<11)|(g<<5)|b; }
  SDL_UpdateTexture(tex,nullptr,conv,SCREEN_W*2);
  SDL_RenderClear(ren); SDL_RenderCopy(ren,tex,nullptr,nullptr); SDL_RenderPresent(ren);
  SDL_Delay(16);
}
void clear(Color c){ for(int i=0;i<SCREEN_W*SCREEN_H;i++) FB[i]=c; }
void pixel(int x,int y,Color c){ if((unsigned)x<(unsigned)SCREEN_W&&(unsigned)y<(unsigned)SCREEN_H) FB[y*SCREEN_W+x]=c; }
void fill_rect(int x,int y,int w,int h,Color c){ for(int j=0;j<h;j++)for(int i=0;i<w;i++) pixel(x+i,y+j,c); }
static std::string mp(const char* p){ std::string s=p,k="/sdcard/"; if(s.rfind(k,0)==0) s="./sdcard_files/"+s.substr(k.size()); return s; }
bool blit_bmp(const char* path){
  SDL_Surface* s=SDL_LoadBMP(mp(path).c_str()); if(!s) return false;
  SDL_Surface* c=SDL_ConvertSurfaceFormat(s,SDL_PIXELFORMAT_RGB24,0); SDL_FreeSurface(s); if(!c) return false;
  unsigned char* px=(unsigned char*)c->pixels;
  for(int y=0;y<c->h&&y<SCREEN_H;y++)for(int x=0;x<c->w&&x<SCREEN_W;x++){ unsigned char* p=px+y*c->pitch+x*3;
    FB[y*SCREEN_W+x]=rgb(p[0],p[1],p[2]); }
  SDL_FreeSurface(c); return true;
}
uint32_t buttons(){ return g_held; }
uint32_t buttons_pressed(){ return g_pressed; }
uint32_t millis(){ return SDL_GetTicks(); }
bool file_exists(const char* path){ FILE* f=fopen(mp(path).c_str(),"rb"); if(f){fclose(f);return true;} return false; }
void return_to_loader(){ g_quit=true; }
void log(const char* m){ printf("[sdl] %s\n",m); }
}

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

// ---------------------------------------------------------------------------
// Audio synthétisé SDL2 (sans SDL_mixer) : 2 voix (music, sfx) mixées dans le
// callback. Mêmes mélodies/effets que l'AKA (le contenu est dans audio/jingle).
// ---------------------------------------------------------------------------
#include <cmath>
namespace gb {
struct Voice {
  volatile bool    on=false;
  volatile float   freq=0;        // Hz
  volatile float   amp=0;         // 0..1
  volatile uint8_t type=TONE_SQUARE;
  volatile uint32_t total=0;      // échantillons de la note
  volatile uint32_t idx=0;        // position courante
  double  phase=0;                // accumulateur de phase (thread audio only)
  uint32_t lfsr=0xACE1u;          // bruit
};
static Voice     s_music, s_sfx;
static SDL_AudioDeviceID s_dev=0;
static const int SR=44100;

static float voice_sample(Voice& v){
  if(!v.on) return 0.f;
  if(v.idx>=v.total){ v.on=false; return 0.f; }
  float t=(float)v.idx/(float)SR;
  float s=0.f;
  switch(v.type){
    case TONE_SINE:   s=sinf(6.28318530718f*v.freq*t); break;
    case TONE_SQUARE: s=(fmodf(v.freq*t,1.f)<0.5f)?1.f:-1.f; break;
    case TONE_TRI:   { float p=fmodf(v.freq*t,1.f); s=4.f*fabsf(p-0.5f)-1.f; } break;
    case TONE_NOISE: { v.lfsr=(v.lfsr>>1)^(-(int)(v.lfsr&1u)&0xB400u); s=((v.lfsr&0xFF)/127.5f)-1.f; } break;
  }
  // enveloppe anti-clic : 3 ms d'attaque / 6 ms de release
  uint32_t atk=SR*3/1000, rel=SR*6/1000;
  float env=1.f;
  if(v.idx<atk) env=(float)v.idx/(float)atk;
  else if(v.idx>v.total-rel) env=(float)(v.total-v.idx)/(float)rel;
  v.idx++;
  return s*v.amp*env;
}

static void audio_cb(void*, Uint8* stream, int len){
  int16_t* out=(int16_t*)stream; int n=len/2;   // mono S16
  for(int i=0;i<n;i++){
    float m=voice_sample(s_music)*0.9f + voice_sample(s_sfx)*0.9f;
    if(m>1.f)m=1.f; if(m<-1.f)m=-1.f;
    out[i]=(int16_t)(m*30000.f);
  }
}

void audio_init(){
  if(s_dev) return;
  if(SDL_InitSubSystem(SDL_INIT_AUDIO)!=0){ std::fprintf(stderr,"audio SDL indisponible: %s\n",SDL_GetError()); return; }
  SDL_AudioSpec want{}, have{};
  want.freq=SR; want.format=AUDIO_S16SYS; want.channels=1; want.samples=1024; want.callback=audio_cb;
  s_dev=SDL_OpenAudioDevice(nullptr,0,&want,&have,0);
  if(!s_dev){ std::fprintf(stderr,"OpenAudioDevice: %s (jeu sans son)\n",SDL_GetError()); return; }
  SDL_PauseAudioDevice(s_dev,0);
}
static void set_voice(Voice& v,float f,float vol,uint16_t ms,uint8_t type){
  if(!s_dev) return;
  SDL_LockAudioDevice(s_dev);
  v.freq=f; v.amp=vol; v.type=type; v.total=(uint32_t)((uint32_t)ms*SR/1000); v.idx=0; v.on=true;
  SDL_UnlockAudioDevice(s_dev);
}
void tone_music(float f,float vol,uint16_t ms,uint8_t type){ set_voice(s_music,f,vol,ms,type); }
void tone_sfx  (float f,float vol,uint16_t ms,uint8_t type){ set_voice(s_sfx,  f,vol,ms,type); }
void audio_stop(){ if(!s_dev)return; SDL_LockAudioDevice(s_dev); s_music.on=false; s_sfx.on=false; SDL_UnlockAudioDevice(s_dev); }
}
