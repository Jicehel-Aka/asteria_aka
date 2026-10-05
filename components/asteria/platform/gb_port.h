// Façade plateforme ASTERIA : SEULE couche liée au matériel AKA.
// Le device implémente ces fonctions dans gb_port_aka.cpp (via gb_graphics/gb_core).
// Le PC les implémente dans gb_port_mock.cpp (pour compiler/tester la logique).
#pragma once
#include <cstdint>
namespace gb {
typedef uint16_t Color;                 // BGR565 (convention AKA)
inline Color rgb(uint8_t r,uint8_t g,uint8_t b){ return (Color)(((b>>3)<<11)|((g>>2)<<5)|(r>>3)); }
constexpr int SCREEN_W=320, SCREEN_H=240;
enum Button:uint32_t{ BTN_UP=1,BTN_DOWN=2,BTN_LEFT=4,BTN_RIGHT=8,BTN_A=16,BTN_B=32,BTN_MENU=64,BTN_RUN=128 };
bool     running();                     // device: true ; mock: budget de frames
void     frame_begin();
void     frame_end();                   // flush framebuffer -> écran
void     clear(Color c);
void     pixel(int x,int y,Color c);
void     fill_rect(int x,int y,int w,int h,Color c);
bool     blit_bmp(const char* path);    // fond plein écran depuis la SD
uint32_t buttons();                     // masque maintenu
uint32_t buttons_pressed();             // front (vient d'être pressé), 1 lecture/frame
uint32_t millis();
bool     file_exists(const char* path);
void     return_to_loader();
void     log(const char* msg);
bool     write_text(const char* path, const char* content);
int      read_text(const char* path, char* out, int maxlen);  // lit un fichier, renvoie la taille (ou -1)
}

namespace gb {
// Blit d'image opaque (rapide sur device via gfx.drawImage) — pour les tuiles.
void draw_image(const uint16_t* px, uint16_t w, uint16_t h, int x, int y);
// Blit avec couleur-clé (transparent) — pour les sprites (peu nombreux).
void draw_image_key(const uint16_t* px, uint16_t w, uint16_t h, int x, int y, uint16_t key);
}
