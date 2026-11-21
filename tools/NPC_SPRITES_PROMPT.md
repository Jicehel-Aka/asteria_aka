# ASTERIA — Prompt de génération des sprites de PNJ (manquants / améliorables)

Complément de `tools/SPRITES_PROMPT.md` (qui couvre les tuiles 16×16). Ici : les
**sprites d'entités** (personnages), qui débordent la case : largeur ~10–14 px ×
**hauteur 20 px**, pieds calés en bas de la case.

## Contraintes techniques (identiques pour chaque personnage)
- Taille cible : **largeur 10–14 px × hauteur 20 px**, personnage centré, **pieds au bord bas**.
- Vue : **top-down 3/4** (de dessus, légèrement inclinée), face vers le bas (le joueur voit l'avant).
- Palette : **BGR565**, 5–8 couleurs par sprite (aplats + 1 ombre + 1 reflet). Pas de dithering agressif.
- Lumière venant du **haut-gauche** (reflets haut-gauche, ombres bas-droite), comme les tuiles.
- Transparence : couleur-clé **magenta `#FF00FF` / `0xF81F`** sur tout le fond.
- Contours nets mais **pas noir pur** (brun/gris foncé), lisibles à 1:1 sur écran 320×240.
- Style : **pixel art médiéval-fantasy** chaleureux, une seule « main », cohérent avec Caïthness.

## Personnages à produire

### Priorité 1 — manquants (actuellement placeholders réutilisés, à remplacer)
- **Racha** — brocanteur/receleur de la Cour des Miracles. Homme sec, la quarantaine,
  manteau râpé à capuche rabattue, ceinture à bourses, un œil méfiant. Tons terreux :
  brun-gris `#5A4E42`, cuir usé `#6E5230`, touche de rouge sombre à l'écharpe. Sournois mais pas monstrueux.
- **Méluve** — habitante démunie de la Cour des Miracles. Jeune femme, châle élimé,
  robe simple gris-bleu `#55606E`, cheveux noués, posture digne malgré la misère.
  Au cou, un **médaillon d'hermine** à peine visible (point crème `#E8E0D0`) — c'est le fil du collier de l'Arc I.

### Priorité 2 — améliorables (procéduraux, un vrai sprite ferait mieux)
- **Cœur de Haine** (boss final, **56×56**, pas 20 px) — masse sombre informe et pulsante,
  cœur rouge battant en son centre (`#B83C32`), veinules rouges rampantes, suintement d'acide
  vert-jaune en bas (`#8AA63C`). Menaçant, organique, asymétrique. Fond magenta.
- **Gardien du Seuil** (lieutenant, **~83×86**) — gardien de pierre/ombre massif barrant l'autel ;
  silhouette lourde, fissures luminescentes froides, pas de visage humain.

### Optionnel — cohérence du casting existant (si tu veux harmoniser)
Caïthness (héroïne, cape bordée d'hermine + dague), Aldren (maire), Lysa (traqueuse),
Maître Albin (érudit), Garrick (Guilde). Mêmes contraintes 20 px de haut.

## Pipeline de réintégration
1. Un **PNG par personnage**, fond `#FF00FF`, dimensions exactes ci-dessus.
2. Baker en tableau C `uint16_t NPC_x[]` (BGR565, clé `0xF81F`) et remplacer l'entrée
   correspondante dans `components/asteria/generated/asteria_npc.h` :
   - Racha = `NPC[18]`, Méluve = `NPC[19]` (remplacer les placeholders `NPC_6` / `NPC_1`,
     mettre la vraie largeur dans `{NPC_18, w, 20}`).
   - Cœur de Haine = `MONSTER[24]` dans `asteria_monster.h` (`{M24, 56, 56}`),
     Gardien = `MONSTER[19]`.
3. **Rebuild hôte** (`./build_pc.sh`) et vérifier le rendu avant commit.
