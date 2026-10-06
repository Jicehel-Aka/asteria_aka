# ASTERIA — Guide de test rapide

## Compiler

### PC (fenêtre SDL) — pour tester confortablement
```bash
cd asteria_aka
bash build_pc.sh          # produit ./asteria_pc (et ./asteria_host pour les captures)
./asteria_pc
```
- **Linux** : `sudo apt install libsdl2-dev`
- **MSYS2 (Windows)** : `pacman -S mingw-w64-x86_64-SDL2` puis lancer depuis le shell MinGW64

### Contenu du paquet de release
Tout le jeu (cartes, sprites, textures donjon, i18n) est **baké dans l'exe**. Pour l'exécuter il faut :
- **Windows** (`ASTERIA_Windows.zip`) : `asteria_pc.exe` + `SDL2.dll` + 3 DLL runtime MinGW
  (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`) + le dossier `sdcard_files/`.
  → tout est déjà dans le zip ; double-cliquer `asteria_pc.exe`.
- **Linux** (`ASTERIA_Linux.zip`) : `asteria_pc` + `sdcard_files/`. SDL2 doit être installé
  sur la machine : `sudo apt install libsdl2-2.0-0`. Lancer depuis le dossier : `./asteria_pc`.
- `sdcard_files/` (écran-titre `TITLE.BMP` + fonds de combat) est **optionnel** : sans lui le jeu
  tourne, mais les fonds sont remplacés par une couleur unie. Il doit être **à côté de l'exe**
  (le jeu cherche `./sdcard_files/...`).

### Console AKA (ESP-IDF)
```bash
cd asteria_aka
idf.py set-target esp32s3
idf.py build flash monitor
```
Prérequis : ta bibliothèque AKA (gb_core/gb_graphics/gb_audio_player) dans `components/gamebuino`,
et `components/asteria/platform/gb_port_aka.cpp` complété (façade gb::).
Les BMP de combat vont sur la SD dans `/sdcard/ASTERIA/battle/`.

## Touches (SDL)
| Action              | Touches |
|---------------------|---------|
| Déplacement / curseur | Flèches ou **Z Q S D** |
| A (valider / action)  | **X** ou **Entrée** |
| B (retour / annuler)  | **C** ou **Retour arrière** |
| MENU                  | Entrée (pavé num.) |
| Quitter la fenêtre    | Échap |

## Parcours de test (boucle complète)
1. **Titre → Création** du perso (répartir les stats, valider avec A).
2. **Valbois** (vue de dessus) : parler aux PNJ (A), boutique (marchand), auberge (porte),
   fontaine, maisons à matériau unique. Sortie **nord** → Forêt.
3. **Forêt** : rencontres aléatoires → **combat tour par tour** (Attaquer / Magie / Objet / Fuir).
   Sortie **nord** → **Donjon (mine)**.
4. **Donjon 1re personne** (raycaster) :
   - ↑ avancer, ↓ reculer, ←/→ tourner (pas et rotation **animés**).
   - Panneau droit : **boussole**, **PV/PM/Or**, **auto-carte** qui se révèle, **journal**.
   - Torches vacillantes, filons (étincelles), flaques d'eau.
   - Trouver le **Chef Gobelin** au fond (A face à lui = combat de boss).
   - **B/MENU** = remonter à la forêt. Après le boss, A sur sa salle = **Grand-Castel**.
5. **Grand-Castel** : enceinte à remparts, tours coniques, PNJ, quête de la Guilde.

## Points à vérifier en priorité
- Framerate du **donjon** sur la vraie console (rendu par pixel chaque frame).
- Lisibilité du **panneau** et de l'**auto-carte** sur écran AKA.
- Enchaînement **combat → retour donjon** (ne doit pas renvoyer en vue de dessus).
- Replacement correct des **PNJ** (aucun sur un toit) et portes/sorties.
- Éditeur intégré (map + sprites) toujours accessible et fonctionnel.

## Éditeur
Accessible depuis le jeu (hub éditeur) : édition de carte (tuiles, départ, spawns,
portails, coffres) et édition de sprites/couleurs. Export via `write_text`/reload.
