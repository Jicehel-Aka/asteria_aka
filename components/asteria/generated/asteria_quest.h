#pragma once
namespace asteria { namespace quest {
struct Quest { const char* name; const char* const* obj; int nobj; bool main; };
static const char* const Q0_O[] = {"Decouvrir le village","Rencontrer le maire","Recevoir la carte locale"};
static const char* const Q1_O[] = {"Retrouver les outils pretes par le forgeron","Rapporter les outils"};
static const char* const Q2_O[] = {"Examiner les empreintes decouvertes par Lysa","Identifier leur origine"};
static const char* const Q3_O[] = {"Identifier les traces demandees par Lysa"};
static const char* const Q4_O[] = {"Recolter 5 herbes medicinales","Recolter 3 champignons"};
static const char* const Q5_O[] = {"Trouver l'indice laisse par un disparu","Identifier la piste menant a la Vieille Mine"};
static const char* const Q6_O[] = {"Explorer la Vieille Mine","Remettre le treuil en fonctionnement","Atteindre la salle d'extraction","Affronter le Chef Gobelin","Recuperer la Clef du Puits"};
static const char* const Q7_O[] = {"Presenter la Pierre noire gravee au bibliothecaire","Identifier son origine","Consulter la bibliotheque specialisee"};
static const char* const Q8_O[] = {"Accomplir trois contrats simples"};
static const char* const Q9_O[] = {"Enqueter sur la caravane disparue","Suivre sa piste vers la Route Royale"};
static const char* const Q10_O[] = {"Examiner le campement abandonne","Trouver le carnet du chef de caravane"};
static const char* const Q11_O[] = {"Enqueter sur Corbeau Noir"};
static const char* const Q12_O[] = {"Aider le marchand a poursuivre sa route"};
static const char* const Q13_O[] = {"Explorer la vallee oubliee","Examiner les monolithes","Utiliser la Pierre noire sur les pierres resonantes","Activer les quatre pierres dans le bon ordre"};
static const char* const Q14_O[] = {"Explorer le Temple Oublie","Resoudre les mecanismes antiques","Atteindre la Salle du Gardien","Vaincre le Gardien du Seuil","Examiner la fresque du sanctuaire","Quitter le temple apres l'effondrement"};
static const int QUEST_COUNT=15;
static const Quest QUEST[] = {
  {"Bienvenue a Valbois",Q0_O,3,true},
  {"Les Outils perdus",Q1_O,2,false},
  {"Les Traces etranges",Q2_O,2,true},
  {"Les empreintes",Q3_O,1,false},
  {"Les plantes de sœur Agnes",Q4_O,2,false},
  {"Le tissu dechire",Q5_O,2,true},
  {"La Vieille Mine",Q6_O,5,true},
  {"Le symbole oublie",Q7_O,3,true},
  {"Les contrats de la Guilde",Q8_O,1,false},
  {"Les marchandises disparues",Q9_O,2,true},
  {"La caravane silencieuse",Q10_O,2,true},
  {"Avis de recherche",Q11_O,1,false},
  {"Une roue cassee",Q12_O,1,false},
  {"Les Pierres Murmurantes",Q13_O,4,true},
  {"Le Temple Oublie",Q14_O,6,true}
};
}
}
