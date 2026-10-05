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
