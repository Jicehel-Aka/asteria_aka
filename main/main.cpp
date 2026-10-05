// main.cpp — Point d'entree de l'app AKA ASTERIA.
// Initialise le materiel (ecran, bus, carte SD) et le socle aka_runtime,
// puis delegue tout le reste au moteur ASTERIA (asteria::run(), qui gere
// sa propre boucle principale via la facade gb::).
#include "gb_core.h"
#include "gb_graphics.h"
#include "gb_audio_player.h"
#include "core/input.h"
#include "aka_runtime/aka_runtime.h"
#include "asteria_app.h"
#include "platform/gb_port.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Instances globales uniques, partagees avec aka_runtime, input et le
// binding materiel d'ASTERIA (platform/gb_port_aka.cpp).
gb_core         g_core;
gb_graphics     gfx;
gb_audio_player g_audio_player;

static void audio_mix_task(void *) {
    for (;;) {
        g_audio_player.pool();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

extern "C" void app_main(void) {
    // 1) Materiel : ecran, bus I2C/ADC, expander, CARTE SD (montee ici), audio.
    g_core.init();
    gfx.set_backlight_percent(80);
    gfx.set_refresh_rate(60);
    input_init();

    // 2) Socle AKA : cree /sdcard/asteria/, charge langue + volumes.
    // BUG EVITE : le moteur (title_scene.cpp, README) reference en dur
    // "/sdcard/ASTERIA/" (majuscules) pour la sauvegarde et TITLE.BMP --
    // l'identifiant ici doit correspondre EXACTEMENT (akaRuntime.begin()
    // cree /sdcard/<id>/), sous peine de chercher au mauvais endroit sur
    // une carte SD sensible a la casse (contrairement a Windows).
    akaRuntime.begin("ASTERIA");
    static auto applyVolume = [](uint8_t musicVol, uint8_t /*sfxVol*/) {
        g_audio_player.set_master_volume((uint8_t)((uint16_t)musicVol * 200 / 100));
    };
    akaRuntime.setVolumeChangedCallback(applyVolume);

    // A ADAPTER si les commandes reelles different (a affiner une fois le
    // deplacement/interactions du monde plus avances).
    static const char *const kControls[] = {
        "CTRL_MOVE",
        "CTRL_CONFIRM",
        "CTRL_MENU_SHORT",
        "CTRL_QUIT",
        nullptr
    };
    akaRuntime.setControlsKeys(kControls);
    akaRuntime.setCredits("Asteria", "jbcebel", "A definir",
                           "");

    g_audio_player.set_master_volume((uint8_t)((uint16_t)akaRuntime.getMusicVolume() * 200 / 100));
    xTaskCreatePinnedToCore(audio_mix_task, "AudioMixTask", 4096, nullptr, 5, nullptr, 1);

    // 3) Delegue tout le reste au moteur -- gere sa propre boucle (scenes,
    // rendu, entrees) via la facade gb::.
    asteria::run();

    // 4) Si le moteur rend la main (scene QUIT), retour au loader.
    gb::return_to_loader();
    while (true) {
        input_poll(g_keys);
        akaRuntime.update(g_keys);
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}
