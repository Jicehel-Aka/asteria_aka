#pragma once
// Sauvegarde / chargement de partie ASTERIA (SAVE.DAT sur la carte SD / sdcard_files).
namespace asteria {
bool game_save();            // ecrit l'etat courant (perso, carte, position, quetes, drapeaux, inventaire)
bool game_load();            // relit SAVE.DAT et restaure l'etat ; false si absent/invalide
extern bool g_autosave;      // false dans les harnais de validation pour ne rien ecrire
}
