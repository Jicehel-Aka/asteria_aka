#pragma once
namespace asteria { namespace evt {
struct Event { const char* id; const char* loc; const char* type; const char* desc; const char* effect; };
static const Event EVENTS[] = {
  {"EVT_VALB","LOC_0001","ambient","Des villageois chuchotent près de la fontaine : encore un disparu dans la forêt.","rien"},
  {"EVT_0001","ARC0_PLAINE","ambient","Une bourrasque traverse la plaine.","rien"},
  {"EVT_0002","ARC0_PLAINE","mystery","Une ombre passe derrière un rocher.","tension"},
  {"EVT_0003","ARC0_PLAINE","encounter","Le Voyageur encapuchonné apparaît au loin.","dialogue:DIA_0100"},
  {"EVT_0100","LOC_0010","wildlife","Un cerf traverse le sentier.","rien"},
  {"EVT_0101","LOC_0010","ambient","Une branche tombe soudainement.","surprise"},
  {"EVT_0102","LOC_0010","weather","Un brouillard dense envahit la zone.","visibility_low"},
  {"EVT_0103","LOC_0010","secret","Une souche creuse cache un petit coffre.","loot:coins_5"},
  {"EVT_0200","LOC_0020","caravan","Une caravane traverse la route.","marchand_temporaire"},
  {"EVT_0201","LOC_0020","weather","Une pluie soudaine ralentit les déplacements.","slow"},
  {"EVT_0202","LOC_0020","encounter","Un garde recherche un fugitif.","dialogue:DIA_0301"},
  {"EVT_0203","LOC_0020","obstacle","Un arbre est tombé sur le chemin.","détour"},
  {"EVT_0300","REG_0003","mystery","Les pierres vibrent faiblement.","hint_resonance"},
  {"EVT_0301","REG_0003","supernatural","Une Ombre errante traverse la vallée.","combat:MON_0015"},
  {"EVT_0302","REG_0003","ambient","Un souffle ancien parcourt les monolithes.","tension"}
};
static const int EVENT_COUNT=15;
}
}
