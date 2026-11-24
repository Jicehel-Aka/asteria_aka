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
int      read_bin (const char* path, void* out, int maxlen);  // lecture BINAIRE (PMF...) : taille lue, ou -1
}

namespace gb {
// Blit d'image opaque (rapide sur device via gfx.drawImage) — pour les tuiles.
void draw_image(const uint16_t* px, uint16_t w, uint16_t h, int x, int y);
// Blit avec couleur-clé (transparent) — pour les sprites (peu nombreux).
void draw_image_key(const uint16_t* px, uint16_t w, uint16_t h, int x, int y, uint16_t key);
// Blit opaque d'une REGION (w x h, coin src (sx,sy)) d'une grande image src_w x src_h.
// Sert a afficher la fenetre visible du cache de carte (OPT-AKA-3).
void blit_region(const uint16_t* src, int src_w, int src_h, int sx, int sy, int w, int h, int dx, int dy);
// Gros tampons (cache de carte, tampon donjon) : PSRAM sur AKA, tas normal sur PC.
void* alloc_big(unsigned bytes);
void  free_big(void* p);
}
#include "platform/gb_raster.h"   // set_target / set_origin / set_clip (etat commun aux ports)
#include "platform/gb_light.h"    // Light, light_row (eclairage jour / nuit)
namespace gb {
// Applique l'eclairage (gb_light.h) aux rangees [y0,y1[ de l'ECRAN deja dessine.
void light_screen(int y0, int y1, int ambient, const Light* lights, int n);
}

namespace gb {
// --- Audio synthétisé (AUCUN fichier requis) ---------------------------------
// Sur AKA : gb_audio_track_tone branché sur g_audio_player (déjà pooled par
//   AudioMixTask dans main.cpp). Sur SDL : voix synthétisées dans le callback
//   audio SDL. Sur host : silencieux (tests).
// Deux voix indépendantes : "music" (mélodie/jingle) et "sfx" (effets courts),
// pour qu'un bip ne coupe pas la musique.
enum ToneType:uint8_t { TONE_SINE=0, TONE_SQUARE=1, TONE_TRI=2, TONE_NOISE=3 };
void audio_init();                                                   // idempotent
void tone_music(float freq, float vol, uint16_t ms, uint8_t type=TONE_SQUARE);
void tone_sfx  (float freq, float vol, uint16_t ms, uint8_t type=TONE_SQUARE);
void audio_stop();                                                   // coupe les 2 voix
// Musique PMF (lecteur de la bibliotheque AKA, gb_audio_track_pmf) : 3e piste, mixee
// avec les 2 voix tone. 'pmf' doit rester valide pendant toute la lecture (boucle).
bool music_play(const void* pmf);                                    // false si indisponible
void music_stop();
void music_volume(float v);                                          // 0..1
}
