#include "ui/text.h"
#include "generated/font_asteria_body.h"
#include "generated/font_asteria.h"
#include "generated/text_style_gold.h"
namespace asteria { namespace ui {
static inline uint8_t idx(unsigned char c){ return (c<FONTB_FIRST)?('?'-FONTB_FIRST):(c-FONTB_FIRST); }
void text(int x,int y,const char* s, gb::Color c){
  for(const unsigned char* p=(const unsigned char*)s; *p; ++p){
    uint8_t g=idx(*p);
    for(int r=0;r<FONTB_HEIGHT;++r){ uint16_t row=FONTB_BITS[g][r];
      for(int col=0;col<16;++col) if(row&(1<<(15-col))) gb::pixel(x+col,y+r,c); }
    x+=FONTB_ADV[g]+1;
  }
}
void text_big(int x,int y,const char* s, gb::Color c){
  for(const unsigned char* p=(const unsigned char*)s; *p; ++p){
    uint8_t g=(*p<FONT_FIRST)?('?'-FONT_FIRST):(*p-FONT_FIRST);
    for(int r=0;r<FONT_HEIGHT;++r){ uint16_t row=FONT_BITS[g][r];
      for(int col=0;col<16;++col) if(row&(1<<(15-col))) gb::pixel(x+col,y+r,c); }
    x+=FONT_ADV[g]+1;
  }
}
int text_w(const char* s){ int w=0; for(const unsigned char* p=(const unsigned char*)s;*p;++p) w+=FONTB_ADV[idx(*p)]+1; return w; }
// dégradé or (par ligne) + contour 4-voisins ; bright=false -> atténué
void gold(int x,int y,const char* s, bool bright){
  gb::Color oc=gb::rgb(GOLD_OUTLINE.r,GOLD_OUTLINE.g,GOLD_OUTLINE.b);
  int x0=x;
  // passe 1 : contour
  for(const unsigned char* p=(const unsigned char*)s; *p; ++p){ uint8_t g=idx(*p);
    for(int r=0;r<FONTB_HEIGHT;++r){ uint16_t row=FONTB_BITS[g][r];
      for(int col=0;col<16;++col) if(row&(1<<(15-col))){
        int px=x+col,py=y+r;
        gb::pixel(px-1,py,oc); gb::pixel(px+1,py,oc); gb::pixel(px,py-1,oc); gb::pixel(px,py+1,oc);
      }}
    x+=FONTB_ADV[g]+1; }
  // passe 2 : remplissage dégradé
  x=x0;
  for(const unsigned char* p=(const unsigned char*)s; *p; ++p){ uint8_t g=idx(*p);
    for(int r=0;r<FONTB_HEIGHT;++r){ uint16_t row=FONTB_BITS[g][r];
      const RGB& gc=GOLD_GRAD[r];
      uint8_t rr=gc.r,gg=gc.g,bb=gc.b;
      if(!bright){ rr=(uint8_t)(rr*0.72f); gg=(uint8_t)(gg*0.72f); bb=(uint8_t)(bb*0.72f); }
      gb::Color c=gb::rgb(rr,gg,bb);
      for(int col=0;col<16;++col) if(row&(1<<(15-col))) gb::pixel(x+col,y+r,c);
    }
    x+=FONTB_ADV[g]+1; }
}
}}
