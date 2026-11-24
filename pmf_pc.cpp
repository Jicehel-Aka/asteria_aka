// ASTERIA (Gamebuino AKA) - Lecteur de musique PMF pour la version PC (SDL).
// Enveloppe le MEME lecteur que la console (components/gamebuino/gb_lib/pmf_player)
// et fournit les fonctions "plateforme" qu'il attend, comme gb_audio_track_pmf.cpp
// le fait sur l'AKA : le mixage est tire par le callback audio SDL (gb_port_sdl.cpp).
#include "pmf_player.h"
#include <cstring>

uint32_t pmf_player::get_sampling_freq(uint32_t sampling_freq_) const { return sampling_freq_; }
void pmf_player::start_playback(uint32_t) {}                  // le callback SDL tire le mixage
void pmf_player::stop_playback() {}
pmf_mixer_buffer pmf_player::get_mixer_buffer(){ pmf_mixer_buffer b = { 0, 0 }; return b; }
void pmf_player::mix_buffer(pmf_mixer_buffer& buf_, unsigned num_samples_){ mix_buffer_impl<int16_t, false, 16>(buf_, num_samples_); }

static pmf_player s_player;
static bool s_on = false;

namespace gb {
// Appeles sous SDL_LockAudioDevice (jamais pendant un mixage).
bool pcmusic_start(const void* pmf, int sr){
  s_player.stop(); s_on = false;
  if (!pmf || std::memcmp(pmf, "pmfx", 4) != 0) return false;  // signature PMF ('pmfx')
  s_player.load(pmf); s_player.start((uint32_t)sr); s_on = true;
  return true;
}
void pcmusic_stop(){ s_player.stop(); s_on = false; }
// Thread audio : remplit buf (mono 16 bits) ; renvoie le nombre d'echantillons ecrits.
int pcmusic_mix(int16_t* buf, int n){
  if (!s_on || !s_player.is_playing()) return 0;
  std::memset(buf, 0, (size_t)n * 2);
  s_player.mix(buf, (unsigned)n);
  return n;
}
}
