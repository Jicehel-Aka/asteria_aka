// Séquenceur de petites mélodies/effets, 100% portable (ne dépend que de la
// façade gb::tone_* et gb::millis). Le contenu musical vit ici, en code partagé,
// pas dans la couche matérielle : AKA, PC et host jouent EXACTEMENT la même chose.
//
// Usage :
//   asteria::audio::init();              // une fois (gb::audio_init + reset)
//   asteria::audio::play(TUNE_DEATH);    // démarre une mélodie
//   asteria::audio::update();            // CHAQUE frame : avance la mélodie
//   asteria::audio::sfx_confirm();       // bip ponctuel (voix sfx séparée)
#pragma once
#include <cstdint>
namespace asteria { namespace audio {

enum Tune : uint8_t { TUNE_NONE=0, TUNE_DEATH, TUNE_VICTORY, TUNE_LEVELUP, TUNE_TITLE };

void init();            // gb::audio_init() + état remis à zéro
void play(Tune t);      // lance une mélodie (coupe la précédente)
void stop();            // coupe la mélodie en cours
void update();          // à appeler chaque frame : déclenche les notes à l'heure
bool playing();         // true tant qu'une mélodie se joue

// Effets courts (voix "sfx", n'interrompent pas la mélodie)
void sfx_select();      // déplacement dans un menu
void sfx_confirm();     // validation
void sfx_coin();        // or / loot
void sfx_hit();         // coup porté
void sfx_hurt();        // coup reçu

}} // namespace asteria::audio
