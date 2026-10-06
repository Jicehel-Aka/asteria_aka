#pragma once
namespace asteria { namespace dlg {
struct Node { const char* require; const char* const* lines; int nlines; const char* set_flag; int give_item; int complete_quest; int start_quest=-1; };
struct NpcDialog { const char* name; const Node* nodes; int nnodes; };
static const char* const D0_0_L[] = {"Ainsi tu es le voyageur. Bienvenue a Valbois.","Tiens, prends cette carte. Tu en auras besoin.","Nos gens disparaissent. J'aurai bientot une requete..."};
static const char* const D0_1_L[] = {"Reviens me voir bientot.","Nous comptons sur toi."};
static const Node D0_N[] = {{"!vu_maire",D0_0_L,3,"vu_maire",5,0},{"",D0_1_L,2,nullptr,-1,-1}};
static const char* const D1_0_L[] = {"Mes outils ont disparu.","Si vous les trouvez pres de la foret, ramenez-les-moi.","Je vous paierai honnetement."};
static const Node D1_N[] = {{"",D1_0_L,3,nullptr,-1,-1}};
static const char* const D2_0_L[] = {"Besoin de quelque chose ?","J'achete aussi ce que vous trouvez.","Les temps sont durs, mais on s'entraide."};
static const Node D2_N[] = {{"",D2_0_L,3,nullptr,-1,-1}};
static const char* const D3_0_L[] = {"Belle journee, n'est-ce pas ?"};
static const Node D3_N[] = {{"",D3_0_L,1,nullptr,-1,-1}};
static const char* const D4_0_L[] = {"Le maire t'a remis la carte ? Parfait.","Repose-toi ici quand tu veux."};
static const char* const D4_1_L[] = {"Un voyageur ? Va donc voir le maire.","Il t'attend, je crois."};
static const Node D4_N[] = {{"vu_maire",D4_0_L,2,nullptr,-1,-1},{"",D4_1_L,2,nullptr,-1,-1}};
static const char* const D5_0_L[] = {"Ces traces... elles ne sont pas humaines.","Quelque chose rode dans la foret.","Soyez prudent."};
static const Node D5_N[] = {{"",D5_0_L,3,"q_traces_ok",-1,2,5}};
static const char* const D6_0_L[] = {"Je ne suis qu'un simple villageois."};
static const Node D6_N[] = {{"",D6_0_L,1,nullptr,-1,-1}};
static const char* const D7_0_L[] = {"On raconte des choses etranges ces temps-ci..."};
static const Node D7_N[] = {{"",D7_0_L,1,nullptr,-1,-1}};
static const char* const D8_0_L[] = {"Les herbes medicinales manquent.","Pouvez-vous m'en rapporter ?","Je vous en serai reconnaissante."};
static const Node D8_N[] = {{"",D8_0_L,3,nullptr,-1,-1}};
static const char* const D9_0_L[] = {"La foret, je la connais depuis toujours.","Ces derniers temps, elle murmure.","Ces traces... trop grandes pour un loup. Sois prudent."};
static const Node D9_N[] = {{"",D9_0_L,3,nullptr,-1,-1}};
static const char* const D10_0_L[] = {"Une caravane a disparu.","Nous devons comprendre ce qu'il s'est passe.","Aidez-nous a enqueter."};
static const Node D10_N[] = {{"",D10_0_L,3,nullptr,-1,9,10}};
static const char* const D13_0_L[] = {"Montrez-moi cette pierre noire...","Hum... fascinant.","Je ne connais pas son origine. Consultez les ouvrages specialises."};
static const Node D13_N[] = {{"",D13_0_L,3,nullptr,-1,-1}};
static const char* const D15_0_L[] = {"Bienvenue a la Guilde.","Accomplissez quelques contrats pour gagner notre confiance.","Nous avons toujours besoin de bras."};
static const Node D15_N[] = {{"",D15_0_L,3,nullptr,-1,-1}};
static const char* const D16_0_L[] = {"Je ne reste jamais longtemps.","Les routes sont pleines de surprises.","Regardez mes marchandises avant que je reparte."};
static const Node D16_N[] = {{"",D16_0_L,3,nullptr,-1,-1}};
static const char* const D17_0_L[] = {"Des remedes ? Des baumes ?","J'ai ce qu'il vous faut.","Les voyageurs parlent d'etranges pierres..."};
static const Node D17_N[] = {{"",D17_0_L,3,nullptr,-1,-1}};
static const NpcDialog DIALOG[18] = {
  {"Aldren",D0_N,2},
  {"Doran",D1_N,1},
  {"Mira",D2_N,1},
  {"Frere Tomas",D3_N,1},
  {"Elian",D4_N,2},
  {"Lysa",D5_N,1},
  {"Rowan",D6_N,1},
  {"Milo",D7_N,1},
  {"Agnes",D8_N,1},
  {"Edric",D9_N,1},
  {"Capitaine Roland",D10_N,1},
  {"Maitre Edrik",nullptr,0},
  {"Sœur Helena",nullptr,0},
  {"Maitre Albin",D13_N,1},
  {"Selene",nullptr,0},
  {"Garrick",D15_N,1},
  {"Tomas Venn",D16_N,1},
  {"Iris",D17_N,1}
};
}
}
