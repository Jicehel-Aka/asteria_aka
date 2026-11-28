// GENERE PAR tools/bake.py
#pragma once
#include <cstdint>
namespace asteria { namespace data {
struct RulesPage{const char* heading;const char* const* lines;uint8_t line_n;};
static const char* const RULES_FR_P0[]={"Tu incarnes un aventurier dans le", "royaume d'Eldor.", "", "Des habitants de Valbois ont disparu", "dans la Forêt de Brume.", "", "Explore, parle aux gens, accepte des", "quêtes. Le monde évolue selon tes", "actions."};
static const char* const RULES_FR_P1[]={"Déplace-toi case par case.", "Parle aux PNJ : indices et quêtes.", "Ouvre les coffres, fouille le décor.", "", "En donjon, garde une source de", "lumière allumée.", "", "Certaines pierres réagissent à la", "Pierre noire — ce sont les Résonances."};
static const char* const RULES_FR_P2[]={"Les combats se jouent au tour par", "tour. À ton tour, choisis :", "  Attaquer   Objet   Fuir", "", "Tous les combats ne sont pas", "obligatoires : certains ennemis", "peuvent être évites.", "", "Repose-toi à l'auberge ou au temple", "pour récupérer tes forces."};
static const char* const RULES_FR_P3[]={"Joystick : se déplacer, naviguer", "A : parler / valider / attaquer", "B : annuler / retour", "MENU : journal et inventaire", "RUN + MENU : quitter vers le lanceur", "", "(Libellés de boutons à confirmer", "selon la console AKA.)"};
static const RulesPage RULES_FR[]={
  {"Le monde",RULES_FR_P0,9},
  {"Exploration",RULES_FR_P1,9},
  {"Combat (tour par tour)",RULES_FR_P2,10},
  {"Contrôles",RULES_FR_P3,8},
};
static constexpr uint8_t RULES_FR_COUNT=4;
} }
