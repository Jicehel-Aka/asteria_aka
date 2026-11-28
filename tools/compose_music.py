#!/usr/bin/env python3
# ASTERIA (Gamebuino AKA) - Compositeur des musiques originales du jeu.
#
# Produit des modules ProTracker 6 voies (.MOD, "6CHN") puis les convertit en PMF
# (format du lecteur de la bibliotheque AKA, gb_audio_track_pmf) avec pmf_converter.
# Tout est genere ici : instruments synthetises (aucun echantillon externe, donc
# aucune question de licence) et partitions ecrites a la main dans les fonctions
# song_*(). Les morceaux sont des boucles (le lecteur rejoue la playlist).
#
# Usage :
#   python3 tools/compose_music.py <dossier_sortie_mod> [chemin/pmf_converter] [dossier_pmf]
# Le .PMF est ce que lit le jeu (sdcard_files/ASTERIA/music/<NOM>.PMF).
#
# Notation des melodies : "A4:4 C5:2 -:2" = note (hauteur reelle, A4 = 440 Hz) :
# duree en rangees. Avec speed 6, 4 rangees = 1 noire au tempo (BPM) du morceau.
import math, os, random, struct, subprocess, sys

RATE = 8287          # frequence de lecture d'un echantillon joue en C-2 (periode 428)
PERIODS = [856,808,762,720,678,640,604,570,538,508,480,453,
           428,404,381,360,339,320,302,285,269,254,240,226,
           214,202,190,180,170,160,151,143,135,127,120,113]
NOTE = {'C':0,'C#':1,'Db':1,'D':2,'D#':3,'Eb':3,'E':4,'F':5,'F#':6,'Gb':6,'G':7,
        'G#':8,'Ab':8,'A':9,'A#':10,'Bb':10,'B':11}

def midi(n):
    name, octv = (n[:-1], int(n[-1]))
    return 12 * (octv + 1) + NOTE[name]

# --------------------------------------------------------------------------------
# Instruments (8 bits signes). base = hauteur MIDI entendue quand on joue C-2.
# Un cycle de N echantillons lu a 8287 Hz donne 8287/N Hz : N=32 -> ~C4 (60).
# --------------------------------------------------------------------------------
class Inst:
    # finetune (-8..7) : la lecture a 8287 Hz (et non 8363) fait sonner un cycle 15,6 cents
    # trop bas ; +1 le remonte (mesure via le lecteur PMF de l'AKA : +5,6 cents).
    # Reglages par instrument mesures par analyse spectrale (voir compose_music.py --diapason).
    def __init__(self, name, data, base, vol=64, loop=None, finetune=1):
        self.name, self.data, self.base, self.vol, self.loop = name, data, base, vol, loop
        self.finetune = finetune

def clip8(v): return max(-127, min(127, int(round(v))))

def cycle(n, fn, amp=110):
    return [clip8(amp * fn(2 * math.pi * i / n)) for i in range(n)]

def inst_flute():
    # sinus + un peu d'harmonique 2 et 3 : doux, "flute de bois"
    d = cycle(32, lambda t: math.sin(t) + 0.18 * math.sin(2 * t) + 0.08 * math.sin(3 * t), 100)
    return Inst("flute", d, 60, 52, (0, 32))

def inst_lead():
    # carre arrondi (combat) : plus de mordant que la flute
    d = cycle(32, lambda t: math.tanh(3.0 * math.sin(t)) * 0.8 + 0.1 * math.sin(3 * t), 100)
    return Inst("lead", d, 60, 44, (0, 32))

def inst_harp():
    # Karplus-Strong : corde pincee, periode 32 -> C4 ; decroissance naturelle, sans boucle
    random.seed(7)
    p = 32; buf = [random.uniform(-1, 1) for _ in range(p)]; out = []
    for i in range(7000):
        v = buf[i % p]; nxt = buf[(i + 1) % p]
        buf[i % p] = 0.4985 * (v + nxt)
        out.append(v)
    peak = max(abs(x) for x in out[:200]) or 1
    # la moyenne de Karplus-Strong raccourcit la periode d'un demi-echantillon : la corde
    # sonne deja +11,7 cents sans correction -> finetune 0 (ecart a la flute : 6 cents)
    return Inst("harp", [clip8(110 * x / peak) for x in out], 60, 48, finetune=0)

def inst_bass():
    # triangle adouci, cycle de 256 -> C-2 = C0. Un cycle long fait jouer les notes graves
    # dans les periodes courtes, ou le pas de lecture 8.8 du lecteur PMF est precis
    # (avec 64 echantillons, un La2 sonnait 37 cents trop bas).
    d = cycle(256, lambda t: (2 / math.pi) * math.asin(math.sin(t)) * 0.85 + 0.15 * math.sin(2 * t), 110)
    return Inst("bass", d, 24, 58, (0, 256))

def inst_pad():
    # 2 dents de scie desaccordees (64 et 65 cycles sur 4096) filtrees : nappe chorus
    n = 4096; raw = []
    for i in range(n):
        a = (i * 64 / n) % 1.0; b = (i * 65 / n) % 1.0
        raw.append((2 * a - 1) + (2 * b - 1))
    sm = []; acc = 0.0
    for x in raw * 2:                       # filtre passe-bas a 1 pole (2 tours : regime etabli)
        acc += 0.18 * (x - acc); sm.append(acc)
    sm = sm[n:]
    peak = max(abs(x) for x in sm) or 1
    return Inst("pad", [clip8(100 * x / peak) for x in sm], 48, 30, (0, n), finetune=0)

def inst_bell():
    # cloche / glockenspiel : partiels inharmoniques, decroissance exponentielle ; N=16 -> C5
    out = []
    for i in range(6000):
        t = i / RATE; f = RATE / 16.0
        v = (math.sin(2 * math.pi * f * t) + 0.45 * math.sin(2 * math.pi * f * 2.76 * t)
             + 0.25 * math.sin(2 * math.pi * f * 5.4 * t))
        out.append(v * math.exp(-t * 3.2))
    peak = max(abs(x) for x in out) or 1
    return Inst("bell", [clip8(115 * x / peak) for x in out], 72, 40)

def inst_kick():
    out = []
    for i in range(1800):
        t = i / RATE; f = 110 * math.exp(-t * 22) + 42
        out.append(math.sin(2 * math.pi * f * t * (1 + 0.0 * t)) * math.exp(-t * 9))
    return Inst("kick", [clip8(120 * x) for x in out], 60, 56)

def inst_snare():
    random.seed(3); out = []
    for i in range(2200):
        t = i / RATE
        v = 0.55 * random.uniform(-1, 1) * math.exp(-t * 18) + 0.45 * math.sin(2 * math.pi * 190 * t) * math.exp(-t * 25)
        out.append(v)
    return Inst("snare", [clip8(115 * x) for x in out], 60, 40)

def inst_hat():
    random.seed(5); out = []; prev = 0
    for i in range(700):
        t = i / RATE; w = random.uniform(-1, 1); hp = w - prev; prev = w   # bruit "aigu"
        out.append(0.6 * hp * math.exp(-t * 60))
    return Inst("hat", [clip8(110 * x) for x in out], 60, 22)

# --------------------------------------------------------------------------------
# Partition : grille de rangees x voies ; chaque cellule = (inst, midi|None, effet, param)
# --------------------------------------------------------------------------------
class Song:
    def __init__(self, name, bpm, speed=6, rows_per_bar=16, bars_per_pattern=4, channels=6):
        self.name, self.bpm, self.speed = name, bpm, speed
        self.rpb, self.bpp, self.ch = rows_per_bar, bars_per_pattern, channels
        self.cells = {}                      # (row, ch) -> [inst, midi, eff, param]
        self.insts = []
        self.length = 0                      # nombre total de rangees utilisees

    def inst(self, i):
        if i not in self.insts: self.insts.append(i)
        return i

    def put(self, row, ch, inst, m, eff=0, param=0):
        self.cells[(row, ch)] = [inst, m, eff, param]
        self.length = max(self.length, row + 1)

    def fx(self, row, ch, eff, param):
        c = self.cells.setdefault((row, ch), [None, None, 0, 0])
        c[2], c[3] = eff, param
        self.length = max(self.length, row + 1)

    def melody(self, ch, inst, text, start=0, vol=None, vib=False):
        """Ecrit une melodie ; '-' = silence (coupe la note). vib : vibrato sur les notes longues."""
        r = start
        for tok in text.split():
            n, d = tok.split(':'); d = int(d)
            if n == '-':
                self.fx(r, ch, 0xC, 0)            # volume 0 = silence
            else:
                self.put(r, ch, inst, midi(n), 0xC if vol is not None else 0, vol or 0)
                if vib and d >= 6:
                    for k in range(3, d):
                        self.fx(r + k, ch, 0x4, 0x32)   # vibrato leger apres l'attaque
            r += d
        self.length = max(self.length, r)
        return r

    def bar_row(self, bar): return bar * self.rpb

# Accords : nom -> notes (midi relatives a la fondamentale)
CHORDS = {'': [0, 4, 7], 'm': [0, 3, 7], '7': [0, 4, 7, 10], 'maj7': [0, 4, 7, 11],
          'm7': [0, 3, 7, 10], 'sus4': [0, 5, 7], 'dim': [0, 3, 6]}

def parse_chord(c):
    root = c[:2] if len(c) > 1 and c[1] in '#b' else c[:1]
    return NOTE[root], CHORDS[c[len(root):]]

def arp(song, ch, inst, prog, octave=4, step=2, shape=(0, 1, 2, 3, 2, 1), start_bar=0, vol=None):
    """Arpege sur chaque mesure de la progression (pas = 'step' rangees)."""
    for b, c in enumerate(prog):
        root, iv = parse_chord(c)
        tones = [12 * (octave + 1) + root + x for x in iv]
        tones = tones + [t + 12 for t in tones]
        r0 = song.bar_row(start_bar + b)
        for k in range(song.rpb // step):
            t = tones[shape[k % len(shape)] % len(tones)]
            song.put(r0 + k * step, ch, inst, t, 0xC if vol else 0, vol or 0)

def bass(song, ch, inst, prog, pattern=((0, 0),), octave=2, start_bar=0):
    """pattern = ((rangee, intervalle), ...) repete chaque mesure ; intervalle 7 = quinte."""
    for b, c in enumerate(prog):
        root, _ = parse_chord(c)
        r0 = song.bar_row(start_bar + b)
        for (r, iv) in pattern:
            song.put(r0 + r, ch, inst, 12 * (octave + 1) + root + iv)

def pad(song, ch, inst, prog, which=1, octave=3, start_bar=0, vol=None):
    for b, c in enumerate(prog):
        root, iv = parse_chord(c)
        song.put(song.bar_row(start_bar + b), ch, inst, 12 * (octave + 1) + root + iv[which % len(iv)], 0xC if vol else 0, vol or 0)

def drums(song, ch, kit, bars, pattern, start_bar=0):
    """pattern : chaine de rpb caracteres : K kick, S snare, h hat, '.' rien."""
    for b in range(bars):
        r0 = song.bar_row(start_bar + b)
        for i, c in enumerate(pattern):
            if c in kit: song.put(r0 + i, ch, kit[c], 60, 0xC if c == 'h' else 0, 26 if c == 'h' else 0)

# --------------------------------------------------------------------------------
# Ecriture du .MOD (ProTracker, 6 voies "6CHN")
# --------------------------------------------------------------------------------
def write_mod(song, path):
    rpp = song.rpb * song.bpp
    assert rpp <= 64
    insts = song.insts
    assert len(insts) <= 31
    npat = (song.length + rpp - 1) // rpp
    # Effets globaux poses sur des voies LIBRES (sans effet) : vitesse + tempo en
    # rangee 0, et D00 (saut au motif suivant) en fin de motif court (< 64 rangees).
    def free_ch(row):
        for c in range(song.ch - 1, -1, -1):
            cell = song.cells.get((row, c))
            if cell is None or cell[2] == 0: return c
        raise SystemExit("aucune voie libre rangee %d (%s)" % (row, song.name))
    c = free_ch(0); song.fx(0, c, 0xF, song.speed)
    c = free_ch(0); song.fx(0, c, 0xF, song.bpm)
    if rpp < 64:
        for p in range(npat):
            r = p * rpp + rpp - 1; c = free_ch(r); song.fx(r, c, 0xD, 0)
    # tempo/speed au tout debut (voie 6 pour ne rien ecraser si possible)
    out = bytearray()
    out += song.name.upper()[:20].ljust(20).encode('ascii')
    for i in range(31):
        if i < len(insts):
            s = insts[i]; data = s.data + ([0] if len(s.data) % 2 else [])
            ln = len(data) // 2
            ls, ll = (s.loop[0] // 2, s.loop[1] // 2) if s.loop else (0, 1)
            out += s.name[:22].ljust(22).encode('ascii') + struct.pack('>HBBHH', ln, s.finetune & 0x0F, s.vol, ls, ll)
        else:
            out += b'\0' * 22 + struct.pack('>HBBHH', 0, 0, 0, 0, 1)
    out += bytes([npat, 127]) + bytes(range(npat)) + bytes(128 - npat) + b'6CHN'
    for p in range(npat):
        for r in range(rpp):
            row = p * rpp + r
            for c in range(song.ch):
                inst, m, eff, prm = song.cells.get((row, c), [None, None, 0, 0])
                period, sn = 0, 0
                if inst is not None and m is not None:
                    idx = 12 + (m - inst.base)
                    while idx < 0: idx += 12
                    while idx > 35: idx -= 12
                    period, sn = PERIODS[idx], insts.index(inst) + 1
                out += bytes([(sn & 0xF0) | ((period >> 8) & 0x0F), period & 0xFF, ((sn & 0x0F) << 4) | eff, prm])
        for r in range(rpp, 64):                                 # rangees jamais atteintes (saut D00)
            out += b'\0' * (4 * song.ch)
    for s in insts:
        data = s.data + ([0] if len(s.data) % 2 else [])
        out += bytes((v & 0xFF) for v in data)
    with open(path, 'wb') as f: f.write(out)
    return npat

# --------------------------------------------------------------------------------
# Les morceaux
# --------------------------------------------------------------------------------
def kit(song):
    return {'K': song.inst(inst_kick()), 'S': song.inst(inst_snare()), 'h': song.inst(inst_hat())}

def song_title():
    """Ecran-titre : 'Le Passage d'Asterion' - la mineur, solennel et lumineux."""
    s = Song("Asteria - Titre", bpm=84)
    fl, hp, bs, pd, bl = (s.inst(f()) for f in (inst_flute, inst_harp, inst_bass, inst_pad, inst_bell))
    kk = s.inst(inst_kick())
    prog = ['Am', 'F', 'C', 'G', 'Am', 'F', 'G', 'E', 'Am', 'F', 'C', 'G', 'F', 'G', 'Am', 'Am']
    s.melody(0, fl, "A4:4 C5:4 E5:8  F5:6 E5:2 D5:4 C5:4  G5:8 E5:4 C5:4  D5:12 B4:4 "
                    "A4:4 C5:4 E5:6 A5:2  A5:8 G5:4 F5:4  E5:4 D5:4 B4:4 G4:4  G#4:8 B4:4 E5:4 "
                    "E5:8 A5:8  G5:6 F5:2 E5:4 D5:4  E5:8 C5:4 E5:4  D5:16 "
                    "C5:4 D5:4 E5:4 F5:4  G5:8 D5:4 B4:4  A4:16  -:16", vib=True)
    arp(s, 1, hp, prog, octave=3, step=2, shape=(0, 1, 2, 3, 4, 3, 2, 1))
    bass(s, 2, bs, prog, ((0, 0), (8, 7)), octave=2)
    pad(s, 3, pd, prog, which=1, octave=3)
    for b in (8, 10, 12, 14):                                   # etincelles de cloche, 2e moitie
        root, iv = parse_chord(prog[b])
        s.put(s.bar_row(b), 4, bl, 72 + root + iv[2])
    for b in range(16):
        if b % 4 == 0: s.put(s.bar_row(b), 5, kk, 60)           # timbale douce en debut de phrase
    return s

def song_village():
    """Valbois de jour : pastorale en sol majeur, 6/8 (12 rangees par mesure)."""
    s = Song("Valbois", bpm=118, rows_per_bar=12, bars_per_pattern=4)
    fl, hp, bs, bl = (s.inst(f()) for f in (inst_flute, inst_harp, inst_bass, inst_bell))
    k = kit(s)
    prog = ['G', 'C', 'G', 'D', 'G', 'C', 'D', 'G', 'Em', 'C', 'G', 'D', 'Em', 'C', 'D', 'G']
    s.melody(0, fl, "D5:4 B4:2 G4:4 B4:2  C5:4 E5:2 G5:6  D5:4 B4:2 D5:4 G5:2  F#5:6 A5:4 F#5:2 "
                    "G5:4 F#5:2 E5:4 D5:2  E5:4 C5:2 E5:4 G5:2  F#5:4 E5:2 D5:4 C5:2  B4:6 G4:6 "
                    "E5:6 G5:4 E5:2  C5:6 E5:6  D5:4 G5:2 B5:6  A5:6 F#5:6 "
                    "G5:4 F#5:2 E5:4 B4:2  C5:4 D5:2 E5:6  D5:4 E5:2 F#5:4 A5:2  G5:12", vib=True)
    arp(s, 1, hp, prog, octave=3, step=2, shape=(0, 1, 3, 2, 1, 3))
    bass(s, 2, bs, prog, ((0, 0), (6, 7)), octave=2)
    for b in range(8, 16, 2):                                   # contre-chant de cloche (partie B)
        root, iv = parse_chord(prog[b])
        s.put(s.bar_row(b) + 6, 3, bl, 72 + root + iv[1])
    drums(s, 5, k, 16, "K.....h.....")
    return s

def song_night():
    """Nocturne : villages et plaine la nuit - mi mineur, lent, cloche + harpe."""
    s = Song("Nocturne", bpm=72)
    bl, hp, bs, pd, fl = (s.inst(f()) for f in (inst_bell, inst_harp, inst_bass, inst_pad, inst_flute))
    prog = ['Em', 'Cmaj7', 'Am', 'B7', 'Em', 'C', 'D', 'B']
    s.melody(0, bl, "G5:8 F#5:4 E5:4  E5:12 G5:4  A5:8 G5:4 E5:4  F#5:12 D#5:4 "
                    "E5:8 G5:4 B5:4  C6:8 B5:4 G5:4  A5:6 F#5:2 D5:8  D#5:8 F#5:4 B4:4")
    s.melody(4, fl, "-:64 E4:16 C5:16 A4:16 B4:16", vol=30, vib=True)  # flute lointaine, 2e tour
    arp(s, 1, hp, prog, octave=3, step=4, shape=(0, 2, 1, 3))
    bass(s, 2, bs, prog, ((0, 0),), octave=2)
    pad(s, 3, pd, prog, which=2, octave=3)
    return s

def song_forest():
    """Foret de Brume : mi dorien, ostinato de harpe, gouttes de cloche, brume de nappe."""
    s = Song("Foret de Brume", bpm=92)
    fl, hp, bs, pd, bl = (s.inst(f()) for f in (inst_flute, inst_harp, inst_bass, inst_pad, inst_bell))
    prog = ['Em', 'D', 'Em', 'D', 'A', 'C', 'D', 'Em'] * 2
    s.melody(0, fl, "-:4 B4:4 E5:8  F#5:4 E5:4 D5:8  -:4 G5:4 F#5:4 E5:4  D5:12 -:4 "
                    "E5:4 C#5:4 A4:8  G4:4 C5:4 E5:8  F#5:8 A5:4 F#5:4  E5:16 "
                    "-:16  -:4 A5:4 G5:4 F#5:4  E5:8 B4:8  D5:12 -:4 "
                    "C#5:4 E5:4 A5:8  G5:4 E5:4 C5:8  D5:4 F#5:4 A5:4 D5:4  E5:16", vib=True)
    arp(s, 1, hp, prog, octave=3, step=2, shape=(0, 2, 1, 2, 3, 2, 1, 2))
    bass(s, 2, bs, prog, ((0, 0),), octave=2)
    pad(s, 3, pd, prog, which=2, octave=3, vol=26)
    random.seed(11)
    scale = [64, 66, 67, 69, 71, 73, 74, 76]                  # mi dorien
    for b in range(16):                                         # gouttes d'eau
        if b % 2: s.put(s.bar_row(b) + random.choice((6, 10, 14)), 4, bl, 12 + random.choice(scale), 0xC, 22)
    return s

def song_mine():
    """Vieille Mine : re phrygien, bourdon grave, battement de coeur, gouttes."""
    s = Song("Vieille Mine", bpm=76)
    fl, bs, pd, bl, hp = (s.inst(f()) for f in (inst_flute, inst_bass, inst_pad, inst_bell, inst_harp))
    kk = s.inst(inst_kick())
    prog = ['Dm', 'Eb', 'Dm', 'C', 'Bb', 'Eb', 'Dm', 'A']
    s.melody(0, fl, "D4:8 F4:4 Eb4:4  D4:12 -:4  A4:8 G4:4 F4:4  E4:12 -:4 "
                    "F4:8 D4:4 F4:4  G4:8 Bb4:4 G4:4  A4:8 F4:4 D4:4  C#4:12 -:4", vol=40, vib=True)
    bass(s, 2, bs, prog, ((0, 0), (8, 0)), octave=1)
    pad(s, 3, pd, prog, which=1, octave=3, vol=24)
    arp(s, 1, hp, prog, octave=2, step=8, shape=(0, 2), vol=30)
    for b in range(8):
        for r in (0, 3):                                        # battement de coeur
            s.put(s.bar_row(b) + r, 5, kk, 60, 0xC, 30 if r else 40)
    random.seed(5)
    for b in range(8):
        s.put(s.bar_row(b) + random.choice((7, 11, 13)), 4, bl, random.choice((74, 77, 81, 86)), 0xC, 18)
    return s

def song_road():
    """Route Royale / Grande Plaine : marche heroique en re majeur."""
    s = Song("Route Royale", bpm=112)
    fl, hp, bs, pd = (s.inst(f()) for f in (inst_flute, inst_harp, inst_bass, inst_pad))
    k = kit(s)
    prog = ['D', 'G', 'D', 'A', 'Bm', 'G', 'A', 'D', 'G', 'A', 'F#m', 'Bm', 'G', 'A', 'D', 'D']
    s.melody(0, fl, "A4:4 D5:4 F#5:6 E5:2  D5:4 G5:4 B5:6 A5:2  F#5:4 D5:4 A4:8  E5:6 F#5:2 E5:4 C#5:4 "
                    "D5:4 F#5:4 B5:8  A5:4 G5:4 F#5:4 E5:4  F#5:4 E5:4 C#5:4 A4:4  D5:12 -:4 "
                    "B5:8 A5:4 G5:4  A5:8 E5:8  F#5:6 E5:2 C#5:4 A4:4  B4:8 D5:4 F#5:4 "
                    "G5:6 F#5:2 E5:4 D5:4  C#5:4 E5:4 A5:8  F#5:8 E5:4 D5:4  D5:16", vib=True)
    arp(s, 1, hp, prog, octave=3, step=2, shape=(0, 1, 2, 1))
    bass(s, 2, bs, prog, ((0, 0), (6, 7), (8, 12), (12, 7)), octave=2)
    pad(s, 3, pd, prog, which=1, octave=3, vol=22)
    drums(s, 5, k, 16, "K.h.S.h.K.h.S.hh")
    return s

def song_combat():
    """Combat : la mineur, 150 BPM, basse en croches, caisse claire sur 2 et 4."""
    s = Song("Combat", bpm=150)
    ld, hp, bs, pd = (s.inst(f()) for f in (inst_lead, inst_harp, inst_bass, inst_pad))
    k = kit(s)
    prog = ['Am', 'Am', 'F', 'G', 'Am', 'Am', 'F', 'E', 'Dm', 'Am', 'F', 'E', 'Dm', 'Am', 'F', 'E']
    s.melody(0, ld, "A4:2 C5:2 E5:2 A5:6 G5:2 E5:2  F5:4 E5:4 D5:2 C5:2 D5:4  C5:6 D5:2 E5:4 F5:4  G5:8 D5:4 B4:4 "
                    "A5:2 G5:2 E5:4 A5:2 G5:2 E5:4  C5:4 D5:4 E5:8  F5:4 E5:4 D5:4 C5:4  B4:8 G#4:8 "
                    "D5:4 F5:4 A5:8  G5:4 E5:4 C5:8  F5:4 A5:4 G5:4 F5:4  E5:8 B4:4 G#4:4 "
                    "A4:4 D5:4 F5:4 A5:4  G5:2 F5:2 E5:4 C5:8  D5:4 E5:4 F5:4 D5:4  E5:8 G#5:8")
    arp(s, 1, hp, prog, octave=4, step=1, shape=(0, 1, 2, 1), vol=28)
    bass(s, 2, bs, prog, tuple((r, 12 if r in (4, 12) else 0) for r in range(0, 16, 2)), octave=1)
    pad(s, 3, pd, prog, which=1, octave=3, vol=20)
    drums(s, 5, k, 16, "K.h.S.hKK.h.S.h.")
    return s

def song_tavern():
    """Taverne : gigue en re majeur, 6/8 sautillant."""
    s = Song("Taverne", bpm=124, rows_per_bar=12, bars_per_pattern=4)
    fl, hp, bs = (s.inst(f()) for f in (inst_flute, inst_harp, inst_bass))
    k = kit(s)
    prog = ['D', 'G', 'D', 'A', 'D', 'G', 'A', 'D'] * 2
    tune = ("A4:2 D5:2 F#5:2 A5:2 F#5:2 D5:2  B5:2 G5:2 D5:2 B4:2 D5:2 G5:2  F#5:2 A5:2 F#5:2 D5:2 E5:2 F#5:2  E5:4 C#5:2 A4:6 "
            "F#5:2 E5:2 D5:2 A5:4 F#5:2  G5:2 B5:2 G5:2 D5:4 B4:2  C#5:2 E5:2 A5:2 G5:2 F#5:2 E5:2  D5:6 -:6 ")
    s.melody(0, fl, tune + tune.replace("D5:6 -:6", "D5:12"))
    arp(s, 1, hp, prog, octave=3, step=2, shape=(0, 1, 2, 0, 1, 2), vol=34)
    bass(s, 2, bs, prog, ((0, 0), (6, 7)), octave=2)
    drums(s, 5, k, 16, "K..h..K..h..")
    return s

SONGS = [("TITRE", song_title), ("VALBOIS", song_village), ("NUIT", song_night), ("FORET", song_forest),
         ("MINE", song_mine), ("ROUTE", song_road), ("COMBAT", song_combat), ("TAVERNE", song_tavern)]

def check_tuning(conv, render):
    """--diapason : joue chaque instrument via le lecteur PMF (rendu WAV) et mesure l'ecart en cents."""
    import numpy as np, tempfile
    tmp = tempfile.mkdtemp()
    for nm, f, notes in [('flute', inst_flute, ('A4',)), ('lead', inst_lead, ('A4',)), ('harp', inst_harp, ('A4',)),
                         ('bass', inst_bass, ('A2', 'D2', 'E1')), ('pad', inst_pad, ('A3',)), ('bell', inst_bell, ('A5',))]:
        for note in notes:
            s = Song('t', bpm=125); i = s.inst(f()); s.put(0, 0, i, midi(note)); s.length = 64
            m, p, w = (os.path.join(tmp, x) for x in ('t.mod', 't.pmf', 't.wav'))
            write_mod(s, m); subprocess.run([conv, '-c', '-i', m, '-o', p], check=True, capture_output=True)
            subprocess.run([render, p, w, '2'], check=True, capture_output=True)
            d = np.frombuffer(open(w, 'rb').read()[44:], dtype=np.int16).astype(float)[1000:23050]
            N = 1 << 20; sp = np.abs(np.fft.rfft(d * np.hanning(len(d)), N)); fr = np.fft.rfftfreq(N, 1 / 44100)
            tgt = 440 * 2 ** ((midi(note) - 69) / 12); band = (fr > tgt * 0.8) & (fr < tgt * 1.25)
            f0 = fr[np.argmax(np.where(band, sp, 0))]
            print('%-6s %-3s %+6.1f cents' % (nm, note, 1200 * math.log2(f0 / tgt)))

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--diapason":
        check_tuning(sys.argv[2], sys.argv[3]); raise SystemExit
    outdir = sys.argv[1] if len(sys.argv) > 1 else "music_src"
    conv = sys.argv[2] if len(sys.argv) > 2 else None
    pmfdir = sys.argv[3] if len(sys.argv) > 3 else outdir
    os.makedirs(outdir, exist_ok=True); os.makedirs(pmfdir, exist_ok=True)
    for name, fn in SONGS:
        s = fn(); mp = os.path.join(outdir, name + ".MOD")
        npat = write_mod(s, mp)
        msg = "%-8s %2d motifs, %2d instruments, MOD %6d o" % (name, npat, len(s.insts), os.path.getsize(mp))
        if conv:
            pp = os.path.join(pmfdir, name + ".PMF")
            r = subprocess.run([conv, "-c", "-i", mp, "-o", pp], capture_output=True, text=True)
            if r.returncode != 0 or not os.path.exists(pp):
                print(r.stdout, r.stderr); raise SystemExit("conversion PMF echouee : " + name)
            msg += " -> PMF %6d o" % os.path.getsize(pp)
        print(msg)
