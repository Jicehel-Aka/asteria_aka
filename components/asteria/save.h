#pragma once
// Sauvegarde / chargement de partie ASTERIA : 3 emplacements (SAVE1..3.DAT, fallback SAVE.DAT).
// Sauvegarde / chargement de partie ASTERIA (SAVE.DAT sur la carte SD / sdcard_files).
namespace asteria {

struct SaveSummary {
  bool exists;
  char name[16];
  int  level;
  int  map;
  int  gold;
};

int  active_save_slot();            // 1..3 (defaut 1)
void set_active_save_slot(int slot);
bool save_slot_summary(int slot, SaveSummary& out);

bool game_save_slot(int slot);
bool game_load_slot(int slot);

bool game_save();                   // sauvegarde sur active_save_slot()
bool game_load();                   // charge active_save_slot()
extern bool g_autosave;             // false dans les harnais de validation
}
