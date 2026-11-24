# ASTERIA – Prompts des icônes d'objets

Les 41 objets du jeu (`OBJ_0001` à `OBJ_0041`) plus une **bourse d'or**, répartis en
**5 planches thématiques**. Une planche de 9 icônes est beaucoup mieux respectée par
les générateurs d'images qu'une grille unique de 42 cases.

Ces icônes serviront à la fenêtre « objet obtenu », au sac de la fenêtre personnage,
à la boutique et à la forge.

## Contraintes techniques (pour que la découpe soit automatique)

- **Fond magenta pur `#FF00FF`**, uni, sans dégradé, sans texture, sans vignettage
  (même méthode que pour les PNJ et les monstres, détourage déjà validé).
- **Grille régulière** : 3 colonnes × 3 lignes (planche 5 : 3 × 2), une icône centrée
  par case, de larges marges magenta entre les cases. Aucun objet ne doit toucher ni
  déborder sur une case voisine.
- **Pas de texte, pas de numéro, pas de cadre, pas de bordure de case**, pas de
  signature ni de filigrane.
- **Pas d'ombre portée au sol** (elle se confond avec le détourage) : un léger ombrage
  sur l'objet lui-même suffit.
- Format carré conseillé : 1024 × 1024 (planches 1 à 4) ou 1024 × 683 (planche 5).
- Je réduis ensuite chaque icône à **24 × 24** (sac, boutique) et **48 × 48**
  (fenêtre « objet obtenu »). Il faut donc des **silhouettes lisibles en tout petit** :
  formes simples, contraste fort, peu de détails fins.

## Style commun (à placer en tête de chaque prompt)

> **Style :** 16-bit fantasy RPG item icons, clean pixel art, crisp 1-pixel dark
> outline around every item, limited warm medieval palette, soft top-left lighting
> with a small highlight, 3/4 view, each object centered in its own cell, consistent
> scale across the sheet, readable silhouette at very small size. Solid flat pure
> magenta background #FF00FF everywhere, no gradient, no texture, no ground shadow,
> no text, no numbers, no frames, no grid lines, generous empty magenta margins
> between cells, nothing touching or overlapping.

---

## Planche 1 – Armes et protections (3 × 3)

Ordre de lecture : de gauche à droite, puis de haut en bas.

```
[Style commun]
A sprite sheet of 9 item icons arranged in a 3x3 grid:
1. a short steel dagger with a leather-wrapped grip and simple crossguard
2. a worn, slightly chipped old iron sword with a dull blade and frayed leather grip
3. a clean, sturdy iron sword with a bright polished blade and brass crossguard
4. a woodcutter's axe: wooden haft, single wide iron blade
5. a small short bow of light wood with a taut string
6. a larger hunter's longbow of dark curved wood with green wrapping on the grip
7. a bundle of three reinforced arrows with iron heads and grey-white fletching, tied with twine
8. a round wooden traveler's shield with an iron rim and a central iron boss
9. a miner's leather helmet with a small brass candle holder on the front
```

| Case | ID | Objet |
|---|---|---|
| 1 | OBJ_0037 | Dague |
| 2 | OBJ_0001 | Épée usée |
| 3 | OBJ_0032 | Épée de fer |
| 4 | OBJ_0002 | Hache de bûcheron |
| 5 | OBJ_0008 | Arc court |
| 6 | OBJ_0020 | Arc de chasseur |
| 7 | OBJ_0027 | Flèches renforcées |
| 8 | OBJ_0023 | Bouclier de voyage |
| 9 | OBJ_0010 | Casque de mineur |

## Planche 2 – Soins, nature et matériaux (3 × 3)

```
[Style commun]
A sprite sheet of 9 item icons arranged in a 3x3 grid:
1. a small round glass flask of red healing potion with a cork stopper
2. a large ornate glass bottle of glowing crimson healing potion with a wax-sealed cork
3. a sprig of green medicinal herb with three serrated leaves, tied with a thin string
4. a forest mushroom with a brown-red speckled cap and a pale stem
5. a small clay honey pot with golden wild honey dripping over the rim
6. a bundle of two dry wooden branches tied together
7. a pile of three black lumps of coal with a faint dull sheen
8. a rusty, pitted iron ingot with orange-brown rust patches
9. a clean grey iron ingot with a smooth metallic shine
```

| Case | ID | Objet |
|---|---|---|
| 1 | OBJ_0005 | Petite potion de soin |
| 2 | OBJ_0033 | Grande potion de soin |
| 3 | OBJ_0034 | Herbe médicinale |
| 4 | OBJ_0035 | Champignon |
| 5 | OBJ_0026 | Miel sauvage |
| 6 | OBJ_0036 | Bois |
| 7 | OBJ_0012 | Charbon |
| 8 | OBJ_0013 | Lingot rouillé |
| 9 | OBJ_0031 | Lingot de fer |

## Planche 3 – Outils, lumière et équipement (3 × 3)

```
[Style commun]
A sprite sheet of 9 item icons arranged in a 3x3 grid:
1. a lit wooden torch with an orange flame and wrapped cloth head
2. a small tin flask of lantern oil with a screw cap and an oil drop symbol shape
3. a sturdy reinforced iron lantern with glass panes and a warm glowing light inside
4. a coiled thin hemp rope
5. a thick, strong coiled rope with an iron grappling ring
6. a worn mining pickaxe with a wooden handle and a chipped iron head
7. a compact traveler's hammer with a short wooden handle
8. a brown leather adventurer's backpack with buckled straps and a rolled blanket on top
9. a folded dark-green hooded rain cape with a bronze clasp
```

| Case | ID | Objet |
|---|---|---|
| 1 | OBJ_0003 | Torche |
| 2 | OBJ_0011 | Huile pour lanterne |
| 3 | OBJ_0019 | Lanterne renforcée |
| 4 | OBJ_0004 | Corde |
| 5 | OBJ_0025 | Corde solide |
| 6 | OBJ_0009 | Pioche usée |
| 7 | OBJ_0021 | Marteau de voyage |
| 8 | OBJ_0018 | Sac d'aventurier |
| 9 | OBJ_0024 | Cape de pluie |

## Planche 4 – Cartes, clés et savoir (3 × 3)

```
[Style commun]
A sprite sheet of 9 item icons arranged in a 3x3 grid:
1. a small rolled parchment village map, partly unrolled, showing tiny houses and a path
2. an old, stained, torn mining map on brown parchment with tunnel lines
3. a large regional map scroll tied with a red ribbon, showing mountains and a river
4. a small leather-bound cartography manual with a compass rose on the cover
5. a magical spellbook with a deep blue cover and four small elemental gems (red, blue, green, white) on it
6. a folded letter sealed with a red wax guild seal
7. a large old iron key with a simple bit, slightly rusty
8. an ornate bronze well key with a ring-shaped bow decorated with a water drop motif
9. a delicate necklace with a white ermine fur pendant on a fine silver chain
```

| Case | ID | Objet |
|---|---|---|
| 1 | OBJ_0006 | Carte de Valbois |
| 2 | OBJ_0016 | Vieille carte minière |
| 3 | OBJ_0017 | Carte régionale de la Marche occidentale |
| 4 | OBJ_0022 | Manuel de cartographie |
| 5 | OBJ_0041 | Livre des Éléments |
| 6 | OBJ_0039 | Pli scellé de la Guilde |
| 7 | OBJ_0007 | Clef de la Vieille Mine |
| 8 | OBJ_0015 | Clef du Puits |
| 9 | OBJ_0038 | Collier d'hermine |

## Planche 5 – Objets mystiques et or (3 × 2)

```
[Style commun]
A sprite sheet of 6 item icons arranged in a grid of 3 columns and 2 rows:
1. a golden ring set with a pale rose-gold gem glowing like the dawn
2. a smooth black stone engraved with faint glowing red runes
3. a broken fragment of an ancient grey stone tablet with carved glyphs
4. a pale blue resonance crystal, faceted and softly glowing, with small light sparkles
5. a dark heart-shaped stone core with pulsing red veins, ominous
6. a small leather purse full of gold coins, a few coins spilling out
```

| Case | ID | Objet |
|---|---|---|
| 1 | OBJ_0040 | Anneau de l'Aube |
| 2 | OBJ_0014 | Pierre noire gravée |
| 3 | OBJ_0028 | Fragment de tablette antique |
| 4 | OBJ_0029 | Cristal de résonance |
| 5 | OBJ_0030 | Cœur de Gardien |
| 6 | — | Bourse d'or (ramassage d'or, récompenses) |

---

## Conseils de génération

- Garde **le même style commun mot pour mot** pour les 5 planches, et si possible la
  même graine (« seed ») : les icônes resteront homogènes entre planches.
- Si un objet sort raté, régénère seulement sa planche, ou demande une planche
  « 1 icône » avec le même style commun : je la remettrai à sa place.
- Si le générateur ajoute des numéros, des cadres ou un fond non magenta, ajoute à la
  fin : « No numbers. No labels. Background must be flat #FF00FF magenta only. »
- Envoie-moi les planches telles quelles (PNG). Je fais la découpe, le détourage, la
  réduction en 24 × 24 et 48 × 48 et l'association aux ID dans l'ordre des tableaux
  ci-dessus.
