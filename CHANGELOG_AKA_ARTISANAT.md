# ASTERIA AKA — Mise a jour Optimisations Console + Artisanat & Objets

Cette archive contient uniquement les fichiers modifies par rapport a la branche
`feat/cartes-liaison-banque-sprites` (`7ddbe95`). Copiez son contenu a la racine
de votre depot `asteria_aka` en ecrasant les fichiers existants.

## 1. Optimisations Gamebuino AKA (ESP32-S3) & PC
- **PSRAM & DRAM (`components/asteria/platform/gb_port_aka.cpp`, `save.cpp`)** :
  deplacement du cache BMP 320x240 (150 Ko) et du tampon de sauvegarde (8 Ko)
  de la DRAM/heap interne vers la PSRAM via `alloc_big()` / `free_big()`.
- **Dirty-frame tracking (`gb_port_aka.cpp`)** : `frame_end()` evite les
  transferts SPI `gfx.update()` redondants sur ecran fixe (synchro >= 500 ms).
- **Audio FreeRTOS (`gb_port_aka.cpp`)** : protection `portMUX_TYPE` autour de
  `s_music_tr` / `s_sfx_tr` face a `AudioMixTask`.
- **Raycaster Donjon (`components/asteria/scene/dungeon_scene.cpp`)** :
  `floorCeil()` rendu sur scanline pile puis copie par `memcpy()` dans le
  framebuffer PSRAM.
- **Durcissement BMP (`gb_port_host.cpp`, `gb_port_sdl.cpp`)** : support BMP
  top-down (`H < 0`) + arret sur lecture tronquee dans Host ; validation des
  dimensions avant `SDL_ConvertSurfaceFormat` dans SDL2.

## 2. Artisanat complet & utilite de tous les objets
- **7 recettes (`components/asteria/generated/asteria_recipes_gen.h`)** :
  1. `Lingot rouille + Charbon` -> `Lingot de fer` [Forge]
  2. `Lingot de fer + Epee usee` -> `Epee de fer` [Forge]
  3. `2 Herbe medicinale + Champignon` -> `Petite potion de soin` [Alchimie portable]
  4. `2 Petite potion + Miel sauvage` -> `Grande potion de soin` [Alchimie portable]
  5. `Lingot de fer + 2 Bois` -> `5x Fleches renforcees` [Forge]
  6. `2 Lingot de fer + 2 Bois` -> `Bouclier de voyage` [Forge]
  7. `Lingot de fer + Huile lanterne` -> `Lanterne renforcee` [Forge]
- **Extraction miniere (`dungeon_scene.cpp`)** : face a un filon dore dans la
  Vieille Mine, appuyer sur `A` extrait `Charbon` + `Lingot rouille` (rendement x2
  avec `Pioche usee` ou `Marteau de voyage`).
- **Cueillette etendue (`world_scene.cpp`)** : ajout de 3 recoltes de `Miel sauvage`
  et 3 recoltes de `Bois` (x2) dans la Foret de Brume + butin d'artisanat sur les monstres.
- **Armes manuelles, armures, fleches, corde & torche (`player.cpp`, `combat_scene.cpp`, `dungeon_scene.cpp`)** :
  - Choix manuel d'arme dans `Pause -> Sac & Equipement` (respecte en combat).
  - Armures passives cumulees : `Bouclier de voyage` (-2 degats), `Casque de mineur` (-1), `Cape de pluie` (-1 et embuscades reduites).
  - `Fleches renforcees` (+2 degats avec arc), `AU` critique et remise -20% en boutique.
  - `Torche` & `Huile pour lanterne` alimentent la lumiere dynamique en Mine (`BTN_RUN`), remplacees a terme par `Lanterne renforcee` (lumiere permanente 100%).
  - `Corde` (usage unique) & `Corde solide` (reutilisable) assurent la fuite en combat ou la sortie rapide de Mine (`BTN_RUN`).

## 3. UI, Sauvegardes & Equilibrage
- **Decodage UTF-8 natif (`components/asteria/ui/text.cpp`)** : les accents de `font_asteria_body.h` et `font_asteria.h` s'affichent sans mojibake.
- **3 emplacements de sauvegarde (`save.h/cpp`, `title_scene.h/cpp`, `world_scene.cpp`)** : `SAVE1..3.DAT` (retrocompatible `SAVE.DAT`), selection Gauche/Droite sur l'ecran-titre ou dans le menu Pause.
- **Menu Pause complet (`world_scene.cpp`)** : `Sac & Equipement`, `Alchimie & Soins`, `Journal & Stats`, `Sauvegarde (3 emplacements)`.
- **Boutique (`shop_scene.cpp`)** : onglets `Acheter / Vendre`, remise `AU`, stock possede et description.
- **Montee de niveau (`progress.cpp`, `combat_scene.cpp`)** : +5 VIT, +2 manaMax, regain de 5 PV et 2 PM ; attitudes (`player.cpp`) +1 PM max tous les 4 PO + regen passive tous les 25 pas.
