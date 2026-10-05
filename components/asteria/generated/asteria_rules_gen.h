// GENERE PAR tools/bake.py
#pragma once
#include <cstdint>
namespace asteria { namespace data {
struct RulesPage{const char* heading;const char* const* lines;uint8_t line_n;};
static const char* const RULES_FR_P0[]={"Tu incarnes un aventurier dans le", "royaume d'Eldor.", "", "Des habitants de Valbois ont disparu", "dans la Foret de Brume.", "", "Explore, parle aux gens, accepte des", "quetes. Le monde evolue selon tes", "actions."};
static const char* const RULES_FR_P1[]={"Deplace-toi case par case.", "Parle aux PNJ : indices et quetes.", "Ouvre les coffres, fouille le decor.", "", "En donjon, garde une source de", "lumiere allumee.", "", "Certaines pierres reagissent a la", "Pierre noire — ce sont les Resonances."};
static const char* const RULES_FR_P2[]={"Les combats se jouent au tour par", "tour. A ton tour, choisis :", "  Attaquer   Objet   Fuir", "", "Tous les combats ne sont pas", "obligatoires : certains ennemis", "peuvent etre evites.", "", "Repose-toi a l'auberge ou au temple", "pour recuperer tes forces."};
static const char* const RULES_FR_P3[]={"Joystick : se deplacer, naviguer", "A : parler / valider / attaquer", "B : annuler / retour", "MENU : journal et inventaire", "RUN + MENU : quitter vers le lanceur", "", "(Libelles de boutons a confirmer", "selon la console AKA.)"};
static const RulesPage RULES_FR[]={
  {"Le monde",RULES_FR_P0,9},
  {"Exploration",RULES_FR_P1,9},
  {"Combat (tour par tour)",RULES_FR_P2,10},
  {"Controles",RULES_FR_P3,8},
};
static constexpr uint8_t RULES_FR_COUNT=4;
} }
