// Implémentation AKA de la façade gb:: — reliée à gb_graphics/gb_core/
// aka_runtime (mêmes composants partagés que tous les autres jeux AKA).
// Ce fichier est le SEUL à dépendre de gb_graphics / gb_core.
#if defined(ESP_PLATFORM)
#include "platform/gb_port.h"

#include "gb_core.h"
#include "gb_graphics.h"
#include "gb_common.h"          // SCREEN_WIDTH/HEIGHT
#include "core/input.h"
#include "aka_runtime/aka_runtime.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Instances globales uniques -- definies dans main.cpp, comme pour tout
// autre jeu AKA (Kong, FirePanic, OilPanic, MrRobot...).
extern gb_core     g_core;
extern gb_graphics gfx;

namespace gb {

// A ADAPTER si les valeurs de bits AKA_KEY_* different sur ta build -- ce
// sont celles verifiees sur tous les autres jeux AKA de ce studio
// (core/input.h, g_keys.raw/pressed/released). Les valeurs gb::Button
// (BTN_UP=1, BTN_A=16, ...) sont DIFFERENTES de la convention materielle
// -- remappage explicite obligatoire, PAS de passage direct possible.
static uint32_t remap_keys(uint32_t raw) {
    uint32_t out = 0;
    if (raw & 0x0200) out |= BTN_UP;     // AKA_KEY_UP
    if (raw & 0x0400) out |= BTN_DOWN;   // AKA_KEY_DOWN
    if (raw & 0x0800) out |= BTN_LEFT;   // AKA_KEY_LEFT
    if (raw & 0x0100) out |= BTN_RIGHT;  // AKA_KEY_RIGHT
    if (raw & 0x8000) out |= BTN_A;      // AKA_KEY_A
    if (raw & 0x2000) out |= BTN_B;      // AKA_KEY_B
    if (raw & 0x0004) out |= BTN_MENU;   // AKA_KEY_MENU
    if (raw & 0x0002) out |= BTN_RUN;    // AKA_KEY_RUN
    return out;
}

bool running() { return true; }

// BUG EVITE (deja rencontre sur un autre jeu AKA en MicroPython, meme
// symptome : "les boutons ne font jamais rien") : rien ne rafraichissait
// l'etat materiel des touches ni ne laissait le menu systeme AKA prendre
// la main. C'est ICI, en debut de frame, que ca doit se faire -- si le
// menu systeme est ouvert (akaRuntime.update() renvoie false), on boucle
// dessus tant qu'il ne s'est pas referme : le moteur (scenes) ne voit
// jamais tourner sa propre logique pendant que le menu a la main.
void frame_begin() {
    input_poll(g_keys);
    while (!akaRuntime.update(g_keys)) {
        vTaskDelay(pdMS_TO_TICKS(16));
        input_poll(g_keys);
    }
}

void frame_end() {
    // Meme regulation de cadence que sur les autres jeux AKA (voir
    // aka_hal.cpp, MrRobot) : l'ecran est documente comme limite a 35 fps
    // (~28ms/image) -- impose un intervalle minimum entre deux
    // rafraichissements reels, quel que soit le rythme d'appel du moteur.
    static uint32_t s_last_ms = 0;
    const uint32_t MIN_INTERVAL_MS = 30;
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
    uint32_t elapsed = now - s_last_ms;
    if (s_last_ms != 0 && elapsed < MIN_INTERVAL_MS) {
        vTaskDelay(pdMS_TO_TICKS(MIN_INTERVAL_MS - elapsed));
    }
    gfx.update();
    s_last_ms = (uint32_t)(esp_timer_get_time() / 1000);
}

void clear(Color c) { gfx.clear(c); }

void pixel(int x, int y, Color c) {
    gfx.setColor(c);
    gfx.drawPixel((int16_t)x, (int16_t)y);
}

void fill_rect(int x, int y, int w, int h, Color c) {
    gfx.setColor(c);
    gfx.fillRect((int16_t)x, (int16_t)y, (int16_t)w, (int16_t)h);
}

// Charge un BMP non compresse (24 bits RGB ou 16 bits BGR565) depuis la SD
// et le dessine plein ecran. Format d'export standard (GIMP/Photoshop/PIL
// "BMP 24 bits" -- pas de compression RLE, pas de palette indexee).
bool blit_bmp(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;

    uint8_t header[54];
    if (fread(header, 1, 54, f) != 54 || header[0] != 'B' || header[1] != 'M') {
        fclose(f);
        return false;
    }
    uint32_t data_offset = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
    int32_t  width       = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
    int32_t  height_raw  = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
    uint16_t bpp          = header[28] | (header[29] << 8);
    uint32_t compression  = header[30] | (header[31] << 8) | (header[32] << 16) | (header[33] << 24);

    bool flip_y = height_raw > 0;              // BMP standard : lignes stockees bas -> haut
    int32_t height = flip_y ? height_raw : -height_raw;

    if (compression != 0 || (bpp != 24 && bpp != 16) || width <= 0 || height <= 0) {
        fclose(f);
        return false;   // format non supporte (compresse, palette indexee, etc.)
    }

    uint16_t* buf = (uint16_t*)malloc((size_t)width * height * sizeof(uint16_t));
    if (!buf) { fclose(f); return false; }

    int row_bytes_src = ((width * (bpp / 8) + 3) / 4) * 4;   // BMP : lignes alignees sur 4 octets
    uint8_t* row = (uint8_t*)malloc(row_bytes_src);
    if (!row) { free(buf); fclose(f); return false; }

    fseek(f, data_offset, SEEK_SET);
    for (int32_t y = 0; y < height; ++y) {
        if (fread(row, 1, row_bytes_src, f) != (size_t)row_bytes_src) {
            free(row); free(buf); fclose(f);
            return false;
        }
        int dst_y = flip_y ? (height - 1 - y) : y;
        uint16_t* dst_row = buf + (size_t)dst_y * width;
        for (int32_t x = 0; x < width; ++x) {
            uint16_t c;
            if (bpp == 24) {
                uint8_t b = row[x * 3 + 0], g = row[x * 3 + 1], r = row[x * 3 + 2];
                c = rgb(r, g, b);
            } else {   // 16 bits : suppose deja BGR565 (meme convention que gb::Color)
                c = (uint16_t)(row[x * 2] | (row[x * 2 + 1] << 8));
            }
            dst_row[x] = c;
        }
    }
    free(row);
    fclose(f);

    gfx.drawImage(0, 0, buf, (uint16_t)width, (uint16_t)height);
    free(buf);
    return true;
}

uint32_t buttons()          { return remap_keys(g_keys.raw); }
uint32_t buttons_pressed()  { return remap_keys(g_keys.pressed); }

uint32_t millis() { return (uint32_t)(esp_timer_get_time() / 1000); }

bool file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

void return_to_loader() { akaRuntime.returnToLoader(); }

void log(const char* msg) { printf("[ASTERIA] %s\n", msg); }

}  // namespace gb

namespace gb {
void draw_image(const uint16_t* px, uint16_t w, uint16_t h, int x, int y){
    gfx.drawImage((int16_t)x, (int16_t)y, px, w, h);        // blit matériel opaque
}
void draw_image_key(const uint16_t* px, uint16_t w, uint16_t h, int x, int y, uint16_t key){
    for(uint16_t j=0;j<h;++j) for(uint16_t i=0;i<w;++i){ uint16_t c=px[j*w+i];
        if(c!=key){ gfx.setColor(c); gfx.drawPixel((int16_t)(x+i),(int16_t)(y+j)); } }
}
}

namespace gb {
bool write_text(const char* path, const char* content){
    FILE* f=fopen(path,"w"); if(!f) return false; fputs(content,f); fclose(f); return true;
}
}

namespace gb {
int read_text(const char* path, char* out, int maxlen){
    FILE* f=fopen(path,"r"); if(!f) return -1; int n=(int)fread(out,1,maxlen-1,f); fclose(f); if(n<0)n=0; out[n]=0; return n;
}
}

// --- Audio synthétisé AKA : 2 pistes tone branchées sur g_audio_player -------
// g_audio_player est défini dans main.cpp et déjà "pooled" par AudioMixTask ;
// on n'ajoute que 2 pistes ton (mélodie + effets). play_tone() ne fait qu'écrire
// quelques champs lus par pool() : note courte, course bénigne tolérée.
#include "gb_audio_track_tone.h"
extern gb_audio_player g_audio_player;
namespace gb {
static gb_audio_track_tone s_music_tr, s_sfx_tr;
static bool s_audio_ready=false;
void audio_init(){
    if(s_audio_ready) return;
    g_audio_player.add_track(&s_music_tr, 0.9f);
    g_audio_player.add_track(&s_sfx_tr,   0.9f);
    s_audio_ready=true;
}
void tone_music(float f,float vol,uint16_t ms,uint8_t type){
    if(!s_audio_ready) return;
    s_music_tr.play_tone(f, vol, ms, (gb_audio_track_tone::tone_type)type);
}
void tone_sfx(float f,float vol,uint16_t ms,uint8_t type){
    if(!s_audio_ready) return;
    s_sfx_tr.play_tone(f, vol, ms, (gb_audio_track_tone::tone_type)type);
}
void audio_stop(){
    if(!s_audio_ready) return;
    s_music_tr.stop_playing(); s_sfx_tr.stop_playing();
}
}
#endif  // ESP_PLATFORM
