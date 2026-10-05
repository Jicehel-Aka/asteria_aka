// GENERE PAR tools/bake.py — NE PAS EDITER
#pragma once
#include <cstdint>
namespace asteria { namespace data {
enum LocType : uint8_t { LT_BUILDING=0, LT_CITY=1, LT_CITY_AREA=2, LT_DUNGEON=3, LT_DUNGEON_AREA=4, LT_FOREST_AREA=5, LT_REGION=6, LT_ROAD_AREA=7, LT_VILLAGE=8, LT_VILLAGE_AREA=9 };
enum ItemCat : uint8_t { IC_AMMO=0, IC_ARMOR=1, IC_CONSUMABLE=2, IC_KEY=3, IC_MAP=4, IC_MATERIAL=5, IC_QUEST=6, IC_TOOL=7, IC_WEAPON=8 };
static constexpr uint16_t NPCF_QUEST_GIVER=1;
static constexpr uint16_t NPCF_MERCHANT=2;
static constexpr uint16_t NPCF_SMITH=4;
static constexpr uint16_t NPCF_HEALER=8;
static constexpr uint16_t NPCF_INNKEEPER=16;
static constexpr uint16_t NPCF_GUILD_MASTER=32;
static constexpr uint16_t NPCF_LIBRARIAN=64;
static constexpr uint16_t NPCF_CARTOGRAPHER=128;
static constexpr uint16_t NPCF_LORE=256;
static constexpr uint16_t ITF_KEY_ITEM=1;
static constexpr uint16_t ITF_STACKABLE=2;
static constexpr uint16_t ITF_UNIQUE=4;
static constexpr uint16_t ITF_NO_SELL=8;
static constexpr uint16_t ITF_LIGHT=16;
static constexpr uint16_t ITF_HEAL=32;
static constexpr uint16_t ITF_RANGED=64;
static constexpr uint16_t ITF_INVENTORY_BONUS=128;
static constexpr uint16_t LOC_COUNT=48;
struct Location{const char* id;const char* name;uint8_t type;int16_t parent;uint16_t conn_off;uint8_t conn_n;};
static const int16_t LOC_CONN[]={8, 31, 0, 14, 8, 24, 31, 38, 0, 24, 38, 24, 39, 38};
static const Location LOCATIONS[LOC_COUNT]={
  {"LOC_0001","Valbois",LT_VILLAGE,-1,0,2},
  {"LOC_0002","Place Centrale",LT_VILLAGE_AREA,0,2,0},
  {"LOC_0003","Auberge du Cerf Blanc",LT_BUILDING,0,2,0},
  {"LOC_0004","Forge de Doran",LT_BUILDING,0,2,0},
  {"LOC_0005","Comptoir de Mira",LT_BUILDING,0,2,0},
  {"LOC_0006","Temple des Saisons",LT_BUILDING,0,2,0},
  {"LOC_0007","Maison du Maire",LT_BUILDING,0,2,0},
  {"LOC_0008","Fermes",LT_VILLAGE_AREA,0,2,0},
  {"LOC_0010","Forêt de Brume",LT_REGION,-1,2,2},
  {"LOC_0011","Entrée Sud",LT_FOREST_AREA,8,4,0},
  {"LOC_0012","Carrefour du Chêne",LT_FOREST_AREA,8,4,0},
  {"LOC_0013","Clairière du Cerf",LT_FOREST_AREA,8,4,0},
  {"LOC_0014","Étang Vert",LT_FOREST_AREA,8,4,0},
  {"LOC_0015","Sentier Oublié",LT_FOREST_AREA,8,4,0},
  {"DNG_0001","Vieille Mine",LT_DUNGEON,-1,4,2},
  {"DNG_0001_01","Entrée",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_02","Galerie principale",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_03","Dépôt",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_04","Atelier",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_05","Ancienne Forge",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_06","Galerie effondrée",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_07","Salle du treuil",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_08","Puits central",LT_DUNGEON_AREA,14,6,0},
  {"DNG_0001_09","Salle d'extraction",LT_DUNGEON_AREA,14,6,0},
  {"CITY_0001","Grand-Castel",LT_CITY,-1,6,2},
  {"CITY_0001_01","Place Centrale",LT_CITY_AREA,24,8,0},
  {"CITY_0001_02","Quartier des Artisans",LT_CITY_AREA,24,8,0},
  {"CITY_0001_03","Marché",LT_CITY_AREA,24,8,0},
  {"CITY_0001_04","Caserne",LT_CITY_AREA,24,8,0},
  {"CITY_0001_05","Bibliothèque",LT_CITY_AREA,24,8,0},
  {"CITY_0001_06","Temple",LT_CITY_AREA,24,8,0},
  {"LOC_0020","Route Royale et Terres Frontalières",LT_REGION,-1,8,3},
  {"LOC_0021","Pont de Pierre",LT_ROAD_AREA,31,11,0},
  {"LOC_0022","Croix des Voyageurs",LT_ROAD_AREA,31,11,0},
  {"LOC_0023","Ancien Verger",LT_ROAD_AREA,31,11,0},
  {"LOC_0024","Camp des Gardes",LT_ROAD_AREA,31,11,0},
  {"LOC_0025","Ruines du Relais",LT_ROAD_AREA,31,11,0},
  {"LOC_0026","Campement abandonné",LT_ROAD_AREA,31,11,0},
  {"REG_0003","Vallée des Pierres Murmurantes",LT_REGION,-1,11,2},
  {"DNG_0002","Temple Oublié",LT_DUNGEON,-1,13,1},
  {"DNG_0002_01","Salle des Colonnes",LT_DUNGEON_AREA,39,14,0},
  {"DNG_0002_02","Hall des Échos",LT_DUNGEON_AREA,39,14,0},
  {"DNG_0002_03","Salle de l'Eau",LT_DUNGEON_AREA,39,14,0},
  {"DNG_0002_04","Salle du Vent",LT_DUNGEON_AREA,39,14,0},
  {"DNG_0002_05","Salle des Glyphes",LT_DUNGEON_AREA,39,14,0},
  {"DNG_0002_06","Crypte Centrale",LT_DUNGEON_AREA,39,14,0},
  {"DNG_0002_07","Sanctuaire",LT_DUNGEON_AREA,39,14,0},
  {"DNG_0002_08","Salle du Gardien",LT_DUNGEON_AREA,39,14,0},
};
static constexpr uint16_t NPC_COUNT=18;
struct Npc{const char* id;const char* name;int16_t home;uint16_t flags;const char* desc;};
static const Npc NPCS[NPC_COUNT]={
  {"NPC_0001","Aldren",6,1,"Maire de Valbois. Convoque le joueur quand les disparitions s'aggravent."},
  {"NPC_0002","Doran",3,7,"Forgeron du village, bourru mais honnête."},
  {"NPC_0003","Mira",4,2,"Tient le comptoir général. Achète les premiers butins."},
  {"NPC_0004","Frère Tomas",5,8,"Soigne gratuitement les blessés au début du jeu."},
  {"NPC_0005","Elian",2,16,"Aubergiste du Cerf Blanc. Connaît presque tout le monde."},
  {"NPC_0006","Lysa",0,1,"Chasseuse qui découvre d'étranges empreintes près de la forêt."},
  {"NPC_0007","Rowan",1,0,"Garde de Valbois, en faction sur la place centrale."},
  {"NPC_0008","Milo",1,0,"Enfant du village, joue sur la place centrale."},
  {"NPC_0009","Agnès",7,1,"Herboriste. Demande des plantes médicinales."},
  {"NPC_0010","Edric",2,0,"Ancien bûcheron qui connaît les récits de la forêt."},
  {"NPC_0011","Capitaine Roland",28,1,"Commande la garnison de Grand-Castel. Recrute des aventuriers."},
  {"NPC_0012","Maître Edrik",26,6,"Forgeron réputé du quartier des artisans."},
  {"NPC_0013","Sœur Helena",30,8,"Grande prêtresse du temple de Grand-Castel."},
  {"NPC_0014","Maître Albin",29,257,"Bibliothécaire. Oriente le joueur vers les bons ouvrages sans donner les réponses."},
  {"NPC_0015","Selene",27,130,"Cartographe de Grand-Castel."},
  {"NPC_0016","Garrick",28,33,"Dirige la Guilde des Aventuriers et le tableau des contrats."},
  {"NPC_0017","Tomas Venn",27,2,"Marchand itinérant, présent certains jours seulement."},
  {"NPC_0018","Iris",27,2,"Apothicaire de Grand-Castel."},
};
static constexpr uint16_t ITEM_COUNT=37;
struct Item{const char* id;const char* name;uint8_t cat;uint16_t flags;int16_t source;};
static const Item ITEMS[ITEM_COUNT]={
  {"OBJ_0001","Épée usée",IC_WEAPON,0,-1},
  {"OBJ_0002","Hache de bûcheron",IC_WEAPON,0,-1},
  {"OBJ_0003","Torche",IC_TOOL,18,-1},
  {"OBJ_0004","Corde",IC_TOOL,2,-1},
  {"OBJ_0005","Petite potion de soin",IC_CONSUMABLE,34,-1},
  {"OBJ_0006","Carte de Valbois",IC_MAP,1,0},
  {"OBJ_0007","Clef de la Vieille Mine",IC_KEY,1,-1},
  {"OBJ_0008","Arc court",IC_WEAPON,64,-1},
  {"OBJ_0009","Pioche usée",IC_TOOL,0,14},
  {"OBJ_0010","Casque de mineur",IC_ARMOR,16,14},
  {"OBJ_0011","Huile pour lanterne",IC_CONSUMABLE,2,14},
  {"OBJ_0012","Charbon",IC_MATERIAL,2,-1},
  {"OBJ_0013","Lingot rouillé",IC_MATERIAL,2,14},
  {"OBJ_0014","Pierre noire gravée",IC_QUEST,5,14},
  {"OBJ_0015","Clef du Puits",IC_KEY,1,14},
  {"OBJ_0016","Vieille carte minière",IC_MAP,0,14},
  {"OBJ_0017","Carte régionale de la Marche occidentale",IC_MAP,1,24},
  {"OBJ_0018","Sac d'aventurier",IC_TOOL,128,24},
  {"OBJ_0019","Lanterne renforcée",IC_TOOL,16,24},
  {"OBJ_0020","Arc de chasseur",IC_WEAPON,64,24},
  {"OBJ_0021","Marteau de voyage",IC_TOOL,0,24},
  {"OBJ_0022","Manuel de cartographie",IC_TOOL,0,24},
  {"OBJ_0023","Bouclier de voyage",IC_ARMOR,0,31},
  {"OBJ_0024","Cape de pluie",IC_ARMOR,0,31},
  {"OBJ_0025","Corde solide",IC_TOOL,2,31},
  {"OBJ_0026","Miel sauvage",IC_CONSUMABLE,34,34},
  {"OBJ_0027","Flèches renforcées",IC_AMMO,2,31},
  {"OBJ_0028","Fragment de tablette antique",IC_QUEST,1,38},
  {"OBJ_0029","Cristal de résonance",IC_QUEST,1,38},
  {"OBJ_0030","Cœur de Gardien",IC_QUEST,13,39},
  {"OBJ_0031","Lingot de fer",IC_MATERIAL,2,-1},
  {"OBJ_0032","Épée de fer",IC_WEAPON,0,-1},
  {"OBJ_0033","Grande potion de soin",IC_CONSUMABLE,34,-1},
  {"OBJ_0034","Herbe médicinale",IC_MATERIAL,2,-1},
  {"OBJ_0035","Champignon",IC_MATERIAL,2,-1},
  {"OBJ_0036","Bois",IC_MATERIAL,2,-1},
  {"OBJ_0037","Dague",IC_WEAPON,0,-1},
};
static constexpr uint16_t MONSTER_COUNT=24;
struct Monster{const char* id;const char* name;uint8_t level;const char* category;};
static const Monster MONSTERS[MONSTER_COUNT]={
  {"MON_0001","Rat des champs",1,"animal"},
  {"MON_0002","Loup gris",1,"animal"},
  {"MON_0003","Corbeau agressif",2,"animal"},
  {"MON_0004","Sanglier sauvage",2,"animal"},
  {"MON_0005","Araignée des broussailles",3,"animal"},
  {"MON_0006","Rat des bois",1,"animal"},
  {"MON_0007","Loup solitaire",1,"animal"},
  {"MON_0008","Araignée des broussailles",3,"animal"},
  {"MON_0009","Sanglier furieux",3,"animal"},
  {"MON_0010","Chef Gobelin",5,"gobelin"},
  {"MON_0011","Brigand",4,"humanoid"},
  {"MON_0012","Archer brigand",4,"humanoid"},
  {"MON_0013","Chien errant",2,"animal"},
  {"MON_0014","Sentinelle de pierre",6,"construct"},
  {"MON_0015","Ombre errante",6,"supernatural"},
  {"MON_0016","Corbeau noir",0,"mysterious"},
  {"MON_0017","Rat des mines",2,"animal"},
  {"MON_0018","Chauve-souris",2,"animal"},
  {"MON_0019","Araignée des cavernes",3,"animal"},
  {"MON_0020","Gardien du Seuil",8,"construct"},
  {"MON_0021","Gardien de Pierre",7,"construct"},
  {"MON_0022","Écho Vivant",6,"supernatural"},
  {"MON_0023","Veilleur Antique",7,"construct"},
  {"MON_0024","Gobelin Fouisseur",3,"gobelin"},
};
} }
