# ASTERIA — Prompt de (re)génération des sprites

But : régénérer les tuiles/sprites d'ASTERIA de façon cohérente, puis les réintégrer via
`tools/bake_sprites.py` (PNG → `components/asteria/generated/asteria_tiles.h`, format BGR565).

## Contraintes techniques (à respecter pour toutes les images)
- Taille : **16×16 px** par tuile (les sprites d'entités peuvent déborder vers le haut, mais la case de base reste 16×16).
- Vue : **top-down légèrement incliné** (vue de dessus 3/4), cohérente entre toutes les tuiles.
- Palette : **BGR565** (profondeur réelle de la console). Peu de couleurs par tuile (4–8), aplats + 1 ombre + 1 reflet.
- Lumière : venant du **haut-gauche** (reflets en haut-gauche, ombres en bas-droite), identique partout.
- Transparence : couleur-clé **magenta `0xF81F`** pour les pixels transparents (contours d'arbres, d'objets).
- Lisibilité à l'échelle 1:1 sur un écran **320×240** : pas de détails sub-pixel, contours nets mais pas noirs purs (brun/gris foncé).
- Style : **pixel art médiéval-fantasy** chaleureux, propre, cohérent (une seule « main »), sans dithering agressif.

## Palette directrice (reprend l'existant)
- Herbe : verts `#3C8A3E`→`#52B050`, brins plus clairs/sombres épars.
- Terre/chemin : bruns `#A8743C` / `#8A5E2E`.
- Pierre/pavé : gris `#96969E` / `#78747A`, joints sombres, mousse `#6078 54` ponctuelle.
- Bois : `#804E28`→`#A2743C`, reflet `#96683C`.
- Fer/ferrures : `#4A4650` / `#787080`, rivets clairs.
- Laiton (serrures, anneaux) : `#E4C670`.
- Toits : tuiles rouges `#9E6034`→`#C8502C`, faîtage crème doux.
- Eau : bleus `#3A6EB4`→`#60BAC4`, rive qui se dissout dans l'herbe (pas de liseré dur).

## Tuiles de terrain / décor à générer (16×16)
1. **herbe** — texture subtile, quelques brins, sans motif répétitif visible.
2. **chemin de terre** — bords irréguliers, touffes d'herbe qui empiètent.
3. **route pavée** — pierres de taille irrégulières, joints, mousse éparse, bords non rectilignes.
4. **eau** — surface ondulée, bords doux se fondant dans l'herbe (rive intermittente).
5. **arbre** — **une seule espèce** (houppier rond dense, tronc brun), débordant vers le haut, cohérent partout.
6. **bosquet** — grappe dense de 2–3 arbres de la MÊME espèce (pas un autre style).
7. **rocher/colline** — relief rocheux brun/gris, facettes, base herbeuse.
8. **porte** — vantail de bois à planches, **arc plein cintre**, 2 bandes de fer + rivets, anneau de laiton, chambranle de pierre.
9. **coffre** — coffre de bois bombé, **ferrures** verticales, **serrure en laiton** + trou, couvercle à reflet.
10. **toits** (rouge/auberge/boutique/mairie), **façades** (pierre/brique/bois/crépi), **fenêtre**, **enseigne**, **rempart**, **tour ronde à toit conique**.
11. Mobilier d'auberge : comptoir, table, tonneau, bouteilles, tabouret.

## Sprites d'entités (héroïne / PNJ / monstres)
- Caïthness (héroïne) : élémentaliste féerique, cape bordée d'hermine, dague ; silhouette lisible 16×≈20.
- PNJ Valbois (maire, villageois), Lysa, Maître Albin, Garrick/Racha, Méluve.
- Monstres : rat des bois, gobelin, Chef Gobelin, et **le Cœur de Haine** (masse sombre informe, veinules rouges, suintement acide).

## Pipeline de réintégration
1. Générer chaque tuile en PNG 16×16 (fond magenta `#FF00FF` pour la transparence).
2. Placer dans le dossier source des PNG, lancer `python tools/bake_sprites.py` → régénère `asteria_tiles.h` (BGR565).
3. Pour des assets venant d'un autre jeu AKA (ex. `Jicehel-Aka/Wargame`), ils sont déjà en 16×16 BGR565 : conversion directe `.bin`→tableau C (cf. tuile `rocher` importée).
4. Rebuild hôte pour vérifier le rendu avant commit.
