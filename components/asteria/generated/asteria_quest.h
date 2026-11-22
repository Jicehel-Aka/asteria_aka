#pragma once
namespace asteria { namespace quest {
struct Quest { const char* name; const char* const* obj; int nobj; bool main; int xp; };
static const char* const Q0_O[] = {"Decouvrir le village de Valbois","Rencontrer le maire Aldren","Recevoir la carte des terres menacees"};
static const char* const Q1_O[] = {"Retrouver les outils pretes par le forgeron","Rapporter les outils"};
static const char* const Q2_O[] = {"Examiner les empreintes trouvees par Lysa","Identifier la marque des Tenebres"};
static const char* const Q3_O[] = {"Identifier les traces demandees par Lysa"};
static const char* const Q4_O[] = {"Recolter 5 herbes medicinales","Recolter 3 champignons"};
static const char* const Q5_O[] = {"Trouver l'indice laisse par un disparu","Suivre la piste jusqu'a la Vieille Mine"};
static const char* const Q6_O[] = {"Explorer la Vieille Mine corrompue","Remettre le treuil en marche","Atteindre la salle d'extraction","Affronter le Chef Gobelin","Recuperer la Clef du Puits"};
static const char* const Q7_O[] = {"Presenter la Pierre noire a Maitre Albin","Identifier la relique des Tenebres","Consulter les traites de la bibliotheque"};
static const char* const Q8_O[] = {"Remettre le pli scelle a Tomas Venn","Fournir 3 herbes medicinales a Iris","Porter une potion a Sœur Helena","Faire son rapport a Garrick"};
static const char* const Q9_O[] = {"Enqueter sur la caravane disparue","Suivre sa piste vers la Route Royale"};
static const char* const Q10_O[] = {"Examiner le campement abandonne","Trouver le carnet du chef de caravane"};
static const char* const Q11_O[] = {"Enqueter sur Corbeau Noir"};
static const char* const Q12_O[] = {"Aider le marchand a poursuivre sa route"};
static const char* const Q13_O[] = {"Explorer la vallee oubliee","Examiner les monolithes","Utiliser la Pierre noire sur les pierres resonantes","Activer les quatre pierres dans le bon ordre"};
static const char* const Q14_O[] = {"Explorer le Temple Oublie","Resoudre les mecanismes antiques","Atteindre la Salle du Gardien","Vaincre le Gardien du Seuil","Examiner la fresque du sanctuaire","Quitter le temple apres l'effondrement"};
static const char* const Q15_O[] = {"Ecouter Meluve a la Cour des Miracles","Retrouver le collier d'hermine aupres de Racha","Rendre le collier a Meluve"};
static const char* const Q16_O[] = {"Rencontrer Dame Orwenn a Valbois","Recevoir l'Anneau de l'Aube"};
static const char* const Q17_O[] = {"Franchir la Breche vers Asteria","Parler au Voyageur Encapuchonne","Gagner le village de Valbois"};
static const int QUEST_COUNT=18;
static const Quest QUEST[] = {
  {"Bienvenue a Valbois",Q0_O,3,true,50},
  {"Les Outils perdus",Q1_O,2,false,70},
  {"Les Traces etranges",Q2_O,2,true,100},
  {"Les empreintes",Q3_O,1,false,60},
  {"Les plantes de sœur Agnes",Q4_O,2,false,80},
  {"Le tissu dechire",Q5_O,2,true,220},
  {"La Vieille Mine",Q6_O,5,true,470},
  {"Le symbole oublie",Q7_O,3,true,150},
  {"Les contrats de la Guilde",Q8_O,4,false,160},
  {"Les marchandises disparues",Q9_O,2,true,150},
  {"La caravane silencieuse",Q10_O,2,true,200},
  {"Avis de recherche",Q11_O,1,false,90},
  {"Une roue cassee",Q12_O,1,false,60},
  {"Les Pierres Murmurantes",Q13_O,4,true,280},
  {"Le Temple Oublie",Q14_O,6,true,400},
  {"Le Collier d'hermine",Q15_O,3,false,180},
  {"L'Anneau de l'Aube",Q16_O,2,false,40},
  {"Le Passage d'Asterion",Q17_O,3,true,30}
};
}
}
