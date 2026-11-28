#pragma once
namespace asteria { namespace dlg {
struct Node { const char* require; const char* const* lines; int nlines; const char* set_flag; int give_item; int complete_quest; int start_quest=-1; };
struct NpcDialog { const char* name; const Node* nodes; int nnodes; };
static const char* const D0_0_L[] = {"Ainsi, c'est toi la voyageuse. Bienvenue à Valbois.","Prends cette carte : les chemins ne sont plus sûrs.","Nos gens disparaissent. Une ombre monte... Reviens me voir."};
static const char* const D0_1_L[] = {"Les Ténèbres rôdent jusqu'aux portes de Valbois.","Découvre ce qui enlève les nôtres. Je t'en conjure.","Mon coffre t'est ouvert, voyageuse : sers-toi, tu en auras besoin."};
static const Node D0_N[] = {{"!vu_maire",D0_0_L,3,"vu_maire",5,0},{"",D0_1_L,3,"maire_coffre",-1,-1}};
static const char* const D1_0_L[] = {"Mes outils ont disparu.","Si vous les trouvez près de la forêt, ramenez-les-moi.","Je vous paierai honnêtement."};
static const Node D1_N[] = {{"",D1_0_L,3,nullptr,-1,-1}};
static const char* const D2_0_L[] = {"Besoin de quelque chose ?","J'achète aussi ce que vous trouvez.","Les temps sont durs, mais on s'entraide."};
static const Node D2_N[] = {{"",D2_0_L,3,nullptr,-1,-1}};
static const char* const D3_0_L[] = {"Belle journée, n'est-ce pas ?"};
static const Node D3_N[] = {{"",D3_0_L,1,nullptr,-1,-1}};
static const char* const D4_0_L[] = {"Le maire t'a remis la carte ? Parfait.","Repose-toi ici quand tu veux."};
static const char* const D4_1_L[] = {"Un voyageur ? Va donc voir le maire.","Il t'attend, je crois."};
static const Node D4_N[] = {{"vu_maire",D4_0_L,2,nullptr,-1,-1},{"",D4_1_L,2,nullptr,-1,-1}};
static const char* const D5_0_L[] = {"Ces empreintes ne sont d'aucune bête connue.","Les Ténèbres gagnent la forêt et enlèvent nos gens.","Suivez leur piste, mais prenez garde à votre âme."};
static const Node D5_N[] = {{"",D5_0_L,3,"q_traces_ok",-1,2,5}};
static const char* const D6_0_L[] = {"Je ne suis qu'un simple villageois."};
static const Node D6_N[] = {{"",D6_0_L,1,nullptr,-1,-1}};
static const char* const D7_0_L[] = {"On raconte des choses étranges ces temps-ci..."};
static const Node D7_N[] = {{"",D7_0_L,1,nullptr,-1,-1}};
static const char* const D8_0_L[] = {"Les herbes médicinales manquent.","Pouvez-vous m'en rapporter ?","Je vous en serai reconnaissante."};
static const Node D8_N[] = {{"",D8_0_L,3,nullptr,-1,-1}};
static const char* const D9_0_L[] = {"La forêt, je la connais depuis toujours.","Ces derniers temps, elle murmure.","Ces traces... trop grandes pour un loup. Sois prudent.","Passe chez moi : mon coffre est à toi, il pourra t'aider."};
static const Node D9_N[] = {{"",D9_0_L,4,"edric_coffre",-1,-1}};
static const char* const D10_0_L[] = {"Une caravane a disparu.","Nous devons comprendre ce qu'il s'est passé.","Aidez-nous à enquêter."};
static const Node D10_N[] = {{"",D10_0_L,3,nullptr,-1,9,10}};
static const char* const D13_0_L[] = {"Cette pierre noire gravée... un froid me saisit.","C'est une relique des Ténèbres, scellée jadis.","Elle porte le sceau du Cœur de Haine. Lisez les vieux traités."};
static const Node D13_N[] = {{"",D13_0_L,3,nullptr,-1,-1}};
// --- Acte III : Guilde des Aventuriers de Grand-Castel (Q8) ---
static const char* const D15_done_L[] = {"Les trois contrats sont honorés. La Guilde te reconnaît des siens.","Repose-toi : d'autres missions viendront."};
static const char* const D15_board_L[] = {"Bienvenue à la Guilde de Grand-Castel. Voici tes contrats :","- Remettre le pli scellé à Tomas Venn.","- Fournir 3 herbes médicinales à Iris.","- Porter une potion à Sœur Helena.","Le pli est déjà dans ta besace. Reviens quand tout sera règle."};
static const char* const D15_intro_L[] = {"La Guilde des Aventuriers siège à Grand-Castel.","Viens m'y trouver si tu cherches des contrats."};
static const Node D15_N[] = {
  {"q8_fait",D15_done_L,2,nullptr,-1,-1},
  {"q8_pris",D15_board_L,5,nullptr,-1,-1},
  {"",D15_intro_L,2,nullptr,-1,-1}
};
static const char* const D16_done_L[] = {"Le pli de la Guilde est bien arrivé. Merci à toi."};
static const char* const D16_q12fait_L[] = {"Ma roue est réparée et la route est sûre. Je repars le cœur léger !","Tiens, pour ta peine. La Guilde avait raison sur toi."};
static const char* const D16_q12_L[] = {"Ma roue est brisée et Corbeau Noir rôde sur la Route Royale.","Je n'ose pas repartir seule. Écarte ce brigand, je t'en prie.","Je saurai te récompenser."};
static const char* const D16_0_L[] = {"Je ne reste jamais longtemps.","Les routes sont pleines de surprises.","Regardez mes marchandises avant que je reparte."};
static const Node D16_N[] = {
  {"q12_fait",D16_q12fait_L,2,nullptr,-1,-1},
  {"q12_pris",D16_q12_L,3,nullptr,-1,-1},
  {"c_pli",D16_done_L,1,nullptr,-1,-1},
  {"",D16_0_L,3,nullptr,-1,-1}
};
static const char* const D17_done_L[] = {"Ces herbes sauveront des vies. La Guilde en sera informée.","Que les tiens soient bénis, voyageuse."};
static const char* const D17_0_L[] = {"Des remèdes ? Des baumes ?","J'ai ce qu'il vous faut.","Si la Guilde m'envoyait des simples, j'en manquerais moins..."};
static const Node D17_N[] = {
  {"c_herbes",D17_done_L,2,nullptr,-1,-1},
  {"",D17_0_L,3,nullptr,-1,-1}
};
static const char* const D12_done_L[] = {"Que la lumière te garde. Nos malades sont soulagés."};
static const char* const D12_0_L[] = {"Tant de misères échouent à la Cour des Miracles...","Une simple potion soulagerait nos plus faibles.","La Guilde a promis son aide. L'apporteras-tu ?"};
static const Node D12_N[] = {
  {"c_aumone",D12_done_L,1,nullptr,-1,-1},
  {"",D12_0_L,3,nullptr,-1,-1}
};
// --- Cour des Miracles de Grand-Castel : Racha (18) et Meluve (19) ---
static const char* const D18_0_L[] = {"L'hermine est repartie chez sa belle. Affaire classée.","Reviens si tu veux vendre, pas mendier."};
static const char* const D18_1_L[] = {"Tu l'as déjà, le bijou. File le rendre avant que je change d'avis."};
static const char* const D18_2_L[] = {"Un collier d'hermine ? Joli, mais sans valeur pour moi.","La fille t'envoie ? Bah... il porte le deuil, pas le profit."};   // suite : dialogue a choix (scene/dialog_choice.cpp)
static const char* const D18_3_L[] = {"Je ne traite qu'avec les gens que je connais.","Reviens quand tu sauras ce que tu cherches."};
static const Node D18_N[] = {
  {"col_rendu",D18_0_L,2,nullptr,-1,-1},
  {"a_collier",D18_1_L,1,nullptr,-1,-1},
  {"vu_meluve",D18_2_L,2,"a_collier",37,-1},
  {"",D18_3_L,2,nullptr,-1,-1}
};
static const char* const D19_0_L[] = {"Tu l'as rendu à sa vraie place. Merci, voyageuse.","Ce blason d'hermine était à ma mère, et avant elle, aux tiens.","Un jour tu sauras ce que cela signifie vraiment."};
static const char* const D19_1_L[] = {"Ce collier ! C'est bien celui de ma famille.","Tu l'as repris à Racha ? Les mots me manquent.","Il est à toi autant qu'à moi, désormais. Garde-le."};
static const char* const D19_2_L[] = {"Approche, on appelle cet endroit la Cour des Miracles.","On m'a pris le seul bien qui me restait : un collier d'hermine.","Racha le brocanteur l'a monnayé. Retrouve-le, je t'en prie."};
static const char* const D19_3_L[] = {"Racha traîne toujours dans son coin, là-haut.","Sans mon collier, je n'ai plus rien des miens."};
static const Node D19_N[] = {
  {"col_rendu",D19_0_L,3,nullptr,-1,-1},
  {"a_collier",D19_1_L,3,"col_rendu",-1,15},
  {"!vu_meluve",D19_2_L,3,"vu_meluve",-1,-1,15},
  {"",D19_3_L,2,nullptr,-1,-1}
};
// --- Dame Orwenn (Valbois) : quete de depart Q16, don de l'Anneau de l'Aube ---
static const char* const D20_gift_L[] = {"Prends l'Anneau de l'Aube, enfant des fées : il était aux tiens.","Il affermira ta main dans tes premiers combats.","Que l'Aube veille sur toi."};
static const char* const D20_intro_L[] = {"Approche, enfant. Je reconnais en toi le sang des fées.","J'ai quelque chose qui t'appartient de droit."};
// DELTA-02 : apres le don, Orwenn ne repete plus "Prends l'Anneau" (flag pose a la 2e visite).
static const char* const D20_after_L[] = {"Garde l'Anneau près de toi, enfant des fées.","Le maire de Valbois a besoin de bras. Va le voir."};
static const Node D20_N[] = {
  {"orwenn_apres",D20_after_L,2,nullptr,-1,-1},
  {"anneau_recu",D20_gift_L,3,nullptr,-1,-1},
  {"",D20_intro_L,2,nullptr,-1,-1}
};
// --- PNJ d'ambiance du hameau : Maitre Edrik (erudit) et Selene (songe) ---
static const char* const D11_0_L[] = {"Je suis Maître Edrik, j'étudie les vieilles pierres de la région.","On dit qu'un Temple dort sous la Vallée, scellé depuis des âges.","Si tu croises des monolithes gravés, écoute-les : ils se souviennent."};
static const Node D11_N[] = {{"",D11_0_L,3,nullptr,-1,-1}};
static const char* const D14_0_L[] = {"Je m'appelle Selene. Je lis parfois ce que la lune murmure.","Tu viens d'ailleurs, fille des fées... et tu cherches les tiens.","Suis le fil de l'hermine : il te mènera plus loin que tu ne crois."};
static const Node D14_N[] = {{"",D14_0_L,3,nullptr,-1,-1}};
// --- Arc 0 : le Voyageur Encapuchonne, Grande Plaine ---
static const char* const D21_0_L[] = {"Ainsi tu as franchi la Brèche, enfant d'Asterion.","Garde le Livre des Éléments de Misris : tes pouvoirs s'y réveillent.","Au sud s'étend Valbois. Cherches-y les tiens ; je veille, de loin."};
static const char* const D21_1_L[] = {"Le vent d'Asteria porte d'étranges présages, voyageuse."};
static const Node D21_N[] = {
  {"vu_voyageur",D21_0_L,3,nullptr,-1,-1},
  {"",D21_1_L,1,nullptr,-1,-1}
};
static const NpcDialog DIALOG[22] = {
  {"Aldren",D0_N,2},
  {"Doran",D1_N,1},
  {"Mira",D2_N,1},
  {"Frère Tomas",D3_N,1},
  {"Elian",D4_N,2},
  {"Lysa",D5_N,1},
  {"Rowan",D6_N,1},
  {"Milo",D7_N,1},
  {"Agnès",D8_N,1},
  {"Edric",D9_N,1},
  {"Capitaine Roland",D10_N,1},
  {"Maître Edrik",D11_N,1},
  {"Sœur Helena",D12_N,2},
  {"Maître Albin",D13_N,1},
  {"Selene",D14_N,1},
  {"Garrick",D15_N,3},
  {"Tomas Venn",D16_N,4},
  {"Iris",D17_N,2},
  {"Racha",D18_N,4},
  {"Meluve",D19_N,4},
  {"Dame Orwenn",D20_N,3},
  {"Voyageur Encapuchonné",D21_N,2}
};
}
}
