# ASTERIA – Images à générer pour la v1.5

J'ai d'abord vérifié ce qui existe déjà (banque PNJ, bestiaire, décors de combat,
icônes des planches 1 à 5, héroïne Caïthness) : tout ce qui suit **manque vraiment**.
Les besoins sont regroupés par image à générer, de la plus utile à la plus facultative.

> **État (v1.5.1)** : les images 1 à 4 sont reçues et intégrées (planche 6, poupée,
> carte `CARTE.BMP`, voyageur : `tools/bake_hero_m.py` + `VOYAG.BMP`). Reste la 5.

| # | Image | Format | Où ça sert | Déjà prévu dans le code ? |
|---|---|---|---|---|
| 1 | Planche 6 : équipement | 3 × 2 cases, fond magenta | Sac, poupée, boutique, forge | Oui : il suffit de déposer `tools/icon_src/planche6.png` |
| 2 | Poupée d'équipement | 1 figure, fond magenta | Fenêtre personnage, onglet Équipement | Oui : `tools/icon_src/poupee.png` |
| 3 | Carte du monde illustrée | 4:3, sans texte | Pause > Carte du monde | Oui : `ASTERIA/CARTE.BMP` sur la carte SD |
| 4 | Voyageur (héros masculin) | Planche 4 directions × 3 frames + portrait | Héros créé au masculin | Non, à brancher à réception |
| 5 | Portraits des PNJ à choix | 3 × 3 bustes | Boîte des dialogues à choix | Non, à brancher à réception (facultatif) |

Contraintes communes (identiques aux planches précédentes) :
- **Fond magenta pur `#FF00FF`**, uni, sans dégradé ni texture (sauf la carte du monde).
- **Pas de texte, pas de numéro, pas de cadre**, pas de signature.
- **Pas d'ombre portée au sol.**

> Remarque de style : les planches 1 et 2 sont en vrai pixel art, les planches 3 à 5 sont
> plus « peintes ». En jeu, une fois réduites en 24 × 24, la différence se voit peu.
> Pour la planche 6, garde le style des planches 3 à 5 (les plus récentes).

---

## 1. Planche 6 – Équipement (3 × 2, la dernière case vide)

Les 5 objets ajoutés pour remplir les emplacements armure, pantalon, bottes et anneau.
En attendant, le jeu affiche une pastille avec l'initiale de l'objet.

```
[Style commun des planches d'icônes]
A sprite sheet of 5 item icons arranged in a grid of 3 columns and 2 rows, the last cell (bottom right) left empty:
1. a brown leather jacket (gambeson style) with laced front and stitched shoulder pads, front view
2. a grey steel chainmail shirt (hauberk) with short sleeves, visible ring texture, front view
3. a pair of reinforced brown leather breeches with small riveted knee plates
4. a pair of sturdy brown leather boots with folded cuffs and buckles, three-quarter view
5. a silver ring set with a deep green gem, with a faint green glow
```

| Case | ID | Objet |
|---|---|---|
| 1 | OBJ_0042 | Veste de cuir |
| 2 | OBJ_0043 | Cotte de mailles |
| 3 | OBJ_0044 | Braies renforcées |
| 4 | OBJ_0045 | Bottes de cuir |
| 5 | OBJ_0046 | Anneau de vigueur |

Intégration : je dépose le PNG sous `tools/icon_src/planche6.png` et je lance
`python3 tools/bake_icons.py`.

## 2. Poupée d'équipement (une figure)

La silhouette affichée derrière les 11 cases (main droite, main gauche, 4 anneaux,
collier, armure, pantalon, bottes, casque). Elle sera réduite à environ **100 × 116 px** :
il faut une figure simple, sombre et lisible, bras légèrement écartés pour que les cases
des mains tombent à côté des hanches.

```
Full-body front view of a neutral hooded traveler mannequin, standing straight and symmetrical,
arms slightly away from the body with open hands at hip height, feet slightly apart.
Simple plain dark-brown cloth clothing with no armor, no weapon, no accessories, face hidden in
the shadow of the hood. Clean 16-bit fantasy RPG pixel art style, crisp 1-pixel dark outline,
muted warm palette, soft top-left lighting, readable silhouette at very small size.
Portrait orientation 2:3 (for example 1024 x 1536), the figure fills the full height.
Solid flat pure magenta background #FF00FF everywhere, no gradient, no ground shadow,
no text, no frame.
```

Intégration : `tools/icon_src/poupee.png`, puis `python3 tools/bake_icons.py`.

## 3. Carte du monde illustrée (une image)

Le jeu dessine déjà une carte « parchemin ». Une illustration la remplace telle quelle :
les noms, chemins, repères et l'étoile d'objectif sont tracés **par le jeu** par-dessus.
L'image ne doit donc contenir **aucun texte**. Les lieux doivent se trouver aux positions
ci-dessous (en % de la largeur et de la hauteur, depuis le coin haut-gauche).

| Lieu | Position | À dessiner |
|---|---|---|
| Grande Plaine | 61 %, 78 % | Prairie ouverte, une faille lumineuse refermée |
| Valbois | 61 %, 63 % | Petit village de bûcherons |
| Forêt de Brume | 57 %, 48 % | Forêt dense, brume légère |
| Vieille Mine | 74 %, 34 % | Entrée de mine dans des collines rocheuses |
| Grand-Castel | 61 %, 21 % | Cité fortifiée, hautes tours |
| Temple Oublié | 82 %, 14 % | Ruines de temple dans les montagnes |
| Route Royale | 34 %, 37 % | Grande route pavée, relais |
| Vallée des Pierres | 17 %, 22 % | Vallée avec des monolithes dressés |

```
Top-down fantasy regional map painted on aged parchment, hand-drawn medieval cartography style,
muted warm colors, no text, no labels, no letters, no compass text.
Composition (4:3, 1600 x 1200): a river flows from the top-left down to the bottom center-right.
Bottom center: open grassland plain with a faint glowing closed rift. Above it: a small woodcutters'
village. Center: a dense misty forest. Center-right, upper: rocky hills with a mine entrance.
Top center: a fortified city with tall towers. Top right corner: mountains with forgotten temple
ruins. Left, middle: a royal paved road with a small roadside inn. Top left: a valley with
standing carved monoliths. Leave the area around each place fairly clear so that small markers
and names can be drawn on top. Thin decorative border, small plain compass rose without letters.
```

Intégration : je la convertis en BMP 320 × 240 sous `ASTERIA/CARTE.BMP`.

## 4. Le Voyageur (héros masculin)

Aujourd'hui, le voyageur créé au masculin utilise le sprite de Caïthness. Il faut le
même format que la planche de l'héroïne : **4 directions × 3 frames de marche**, plus un
portrait pour l'écran de création (comme `CAITH.BMP`).

**4a. Planche de marche**

```
Sprite sheet of a young male fantasy traveler for a top-down 16-bit RPG, same proportions and
style as a classic SNES RPG hero: hooded dark navy-blue cloak, brown leather jerkin, belt pouch,
short dagger at the hip, brown boots. 4 rows (walking down, up, left, right) x 3 columns
(walk frames: left foot, standing, right foot), each frame centered in its own cell with generous
empty margins, identical scale on every frame. Clean pixel art, crisp 1-pixel dark outline,
soft top-left lighting. Solid flat pure magenta background #FF00FF, no ground shadow, no text,
no grid lines.
```

**4b. Portrait de création**

```
Half-body portrait of the same young male traveler (hooded dark navy cloak, brown leather jerkin),
three-quarter view, determined calm expression, fantasy painted illustration style matching a
16-bit RPG title screen, warm dusk lighting, plain dark background, no text, 4:3 (1600 x 1200).
```

## 5. Portraits des PNJ à choix (facultatif)

Les 7 PNJ qui proposent un dialogue à choix, plus 2 PNJ très visités. Ils seraient
affichés en petit (48 × 48) dans la boîte de dialogue. Les descriptions reprennent les
tenues et couleurs des sprites de la banque PNJ. Exception : Meluve, dont le sprite actuel
ressemble à un vieil homme alors que le récit en fait une jeune fille (« la fille
t'envoie ? ») ; le portrait suit le récit.

```
[Style commun des planches d'icônes, en remplaçant "item icons" par "character bust portraits"]
A sprite sheet of 9 fantasy RPG character bust portraits (head and shoulders, facing the viewer),
arranged in a 3x3 grid:
1. a mysterious traveler in a grey-blue hooded cloak, face hidden in deep shadow
2. an elderly village mayor with a long grey beard and a deep crimson robe
3. a young huntress in a green hood and leather armor, a bow over the shoulder
4. a grey-haired veteran guild master with a fur-collared dark coat and a red cloak
5. a travelling merchant woman with a wide-brimmed brown hat and a long brown coat
6. a young noble girl in a worn ermine-trimmed cloak, sad but hopeful eyes
7. a shifty junk dealer with a patched wide-brimmed hat, a crooked smile and small trinkets
8. a kind grey-haired herbalist in a blue-green dress, holding a sprig of herbs
9. a burly bald blacksmith with stubble and a brown leather apron
```

| Case | PNJ |
|---|---|
| 1 | Voyageur Encapuchonné |
| 2 | Aldren (maire) |
| 3 | Lysa |
| 4 | Garrick |
| 5 | Tomas Venn |
| 6 | Meluve |
| 7 | Racha |
| 8 | Agnès |
| 9 | Doran |

---

### Ordre conseillé

1, 2 et 3 sont directement pris en charge par le jeu : il suffit de me renvoyer les images.
4 compte le plus pour le joueur qui crée un voyageur. 5 est un plus.
