// ASTERIA (Gamebuino AKA) - Dialogues a choix par attitude (voir dialog_choice.h).
#include "scene/dialog_choice.h"
#include "ui/text.h"
#include "ui/item_ui.h"
#include "platform/gb_port.h"
#include "gamestate.h"
#include "content_i18n.h"
#include "audio/jingle.h"
#include "player.h"
#include <cstdio>
namespace asteria { namespace choice {

enum Att : uint8_t { AGR, AMI, RUS, PRU };   // ordre de Character::attitude
static const char* const ATT_NAME[4] = { "Agressive", "Amicale", "Rusée", "Prudente" };

struct Option {
  uint8_t att; const char* text; const char* reply;
  int16_t gold;         // or gagne (>0)
  int16_t cost;         // or a payer ; si le joueur n'a pas assez : fail_reply, rien d'autre
  const char* item; uint8_t qty;   // objet donne par le PNJ (OBJ_xxxx) ou nullptr
  const char* fail_reply;
};
struct Set { int8_t npc, node; const char* done_flag; Option opt[4]; };

// Textes en francais (cles de traduction, voir tools/gen_i18n.py)
static const Set SETS[] = {
  // Voyageur Encapuchonne, a la sortie de la Breche : premiere attitude du heros
  {21, 0, "ch_voyageur", {
    {AGR, "Je n'ai besoin de personne.", "Cette fierté te gardera debout... ou te brisera.", 0, 0, nullptr, 0, nullptr},
    {AMI, "Merci, ami. Ta présence me rassure.", "Tu as plus d'alliés ici que tu ne le crois.", 0, 0, nullptr, 0, nullptr},
    {RUS, "Tu sembles en savoir beaucoup sur moi...", "Ta famille est passée par Valbois. Le reste, à toi de le trouver.", 0, 0, nullptr, 0, nullptr},
    {PRU, "Quels dangers m'attendent sur la route ?", "Loups en forêt, gobelins dans la mine. Repose-toi à l'auberge.", 0, 0, nullptr, 0, nullptr} }},
  // Maire Aldren, premiere rencontre (Q0)
  {0, 0, "ch_maire", {
    {AGR, "Je chasserai ces Ténèbres, moi !", "Que ta lame soit aussi vive que ta langue.", 0, 0, nullptr, 0, nullptr},
    {AMI, "Valbois peut compter sur moi.", "Merci. Le village ne l'oubliera pas.", 0, 0, nullptr, 0, nullptr},
    {RUS, "Et qu'y gagnerai-je, monsieur le maire ?", "Ha ! Voici 15 pièces d'avance. Le reste à ton retour.", 15, 0, nullptr, 0, nullptr},
    {PRU, "Que savez-vous de ces disparitions ?", "Toujours près de la forêt, la nuit. Lysa a vu d'étranges traces.", 0, 0, nullptr, 0, nullptr} }},
  // Lysa et les traces (Q2)
  {5, 0, "ch_lysa", {
    {AGR, "Qu'elles viennent, ces bêtes !", "Brave... mais ce ne sont pas des bêtes ordinaires.", 0, 0, nullptr, 0, nullptr},
    {AMI, "Je retrouverai vos disparus, je le promets.", "Alors prends ces herbes. La forêt use les imprudents.", 0, 0, "OBJ_0034", 2, nullptr},
    {RUS, "Personne d'autre n'a rien remarqué ?", "Edric, le vieux bûcheron. Il en sait plus qu'il ne dit.", 0, 0, nullptr, 0, nullptr},
    {PRU, "Où mènent ces traces, exactement ?", "Vers le nord de la forêt... jusqu'à la Vieille Mine.", 0, 0, nullptr, 0, nullptr} }},
  // Garrick, tableau des contrats de la Guilde (Q8)
  {15, 1, "ch_garrick", {
    {AGR, "Des corvées ? Je vaux mieux que ça.", "Alors prouve-le en les finissant vite.", 0, 0, nullptr, 0, nullptr},
    {AMI, "Avec plaisir, comptez sur moi.", "Voilà qui fait plaisir à entendre.", 0, 0, nullptr, 0, nullptr},
    {RUS, "Une petite avance m'aiderait beaucoup...", "Hmph. 15 pièces, et pas une de plus.", 15, 0, nullptr, 0, nullptr},
    {PRU, "Quels dangers m'attendent en ville ?", "La Cour des Miracles grouille de coupe-bourses. Méfie-toi.", 0, 0, nullptr, 0, nullptr} }},
  // Tomas Venn et sa roue cassee (Q12)
  {16, 1, "ch_tomas", {
    {AGR, "Ce Corbeau Noir va regretter d'être né.", "Puisses-tu dire vrai. Je t'attendrai ici.", 0, 0, nullptr, 0, nullptr},
    {AMI, "Je m'occupe de ce brigand, ne craignez rien.", "Merci ! Prends cette potion, pour la route.", 0, 0, "OBJ_0005", 1, nullptr},
    {RUS, "Et que m'offrez-vous en échange ?", "Quarante pièces d'or, parole de marchand.", 0, 0, nullptr, 0, nullptr},
    {PRU, "Combien de brigands l'accompagnent ?", "Une poignée, et des archers. Garde tes distances.", 0, 0, nullptr, 0, nullptr} }},
  // Meluve, a la Cour des Miracles (Q15)
  {19, 2, "ch_meluve", {
    {AGR, "Racha le rendra, de gré ou de force.", "Ne verse pas de sang pour moi... mais rapporte-le.", 0, 0, nullptr, 0, nullptr},
    {AMI, "Je te le rapporterai, je te le promets.", "Ta bonté me rend l'espoir.", 0, 0, nullptr, 0, nullptr},
    {RUS, "Et si Racha refuse de le rendre ?", "Il est superstitieux. Parle-lui de malheur, il cédera.", 0, 0, nullptr, 0, nullptr},
    {PRU, "Où puis-je trouver ce Racha ?", "Là-haut, dans le coin du brocanteur. Garde ta bourse fermée.", 0, 0, nullptr, 0, nullptr} }},
  // Racha le brocanteur et le collier d'hermine (Q15) : le collier n'est donne que si
  // le choix aboutit (payer 20 or sans les avoir : rien, on peut revenir)
  {18, 2, "ch_racha", {
    {AGR, "Donne-le-moi, ou tu le regretteras.", "Doucement ! Le voilà, et file avant que j'appelle la garde.", 0, 0, nullptr, 0, nullptr},
    {AMI, "Meluve n'a plus rien. Aie du cœur.", "Tiens. Et qu'on ne dise pas que Racha n'a pas de cœur.", 0, 0, nullptr, 0, nullptr},
    {RUS, "Ce blason porte malheur à qui le vend...", "Malheur ? Prends-le, et emporte ta guigne avec toi !", 0, 0, nullptr, 0, nullptr},
    {PRU, "Combien en veux-tu ? (20 or)", "Vingt pièces. Marché conclu.", 0, 20, nullptr, 0, "Pas d'or, pas de collier. Reviens plus riche."} }},
};
static const int NSETS = (int)(sizeof(SETS) / sizeof(SETS[0]));

static const Set* s_set = nullptr;
static int  s_sel = 0;
static bool s_reply = false;     // la reponse du PNJ est affichee
static bool s_accept = true;
static const char* s_text = nullptr;
static char s_extra[64];         // effet gagne (affiche sous la reponse)

void reset(){ s_set = nullptr; s_reply = false; }
bool active(){ return s_set != nullptr; }

bool begin(int npc, int node){
  for(int i = 0; i < NSETS; ++i){
    const Set& S = SETS[i];
    if(S.npc == npc && S.node == node && !flag_get(S.done_flag)){
      s_set = &S; s_sel = 0; s_reply = false; s_accept = true; s_extra[0] = 0;
      return true;
    }
  }
  return false;
}

Result update(uint32_t p){
  if(!s_set) return DONE_ACCEPT;
  if(s_reply){
    if(p & (gb::BTN_A | gb::BTN_B)){ s_set = nullptr; s_reply = false; return s_accept ? DONE_ACCEPT : DONE_DECLINE; }
    return RUNNING;
  }
  if(p & gb::BTN_DOWN) s_sel = (s_sel + 1) % 4;
  if(p & gb::BTN_UP)   s_sel = (s_sel + 3) % 4;
  if(!(p & gb::BTN_A)) return RUNNING;
  const Option& o = s_set->opt[s_sel]; Character& c = player();
  s_extra[0] = 0;
  if(o.cost > 0 && c.gold < o.cost){          // pas assez d'or : le choix echoue, on pourra revenir
    s_text = o.fail_reply; s_accept = false; s_reply = true; audio::sfx_error();
    return RUNNING;
  }
  add_attitude(c, o.att);
  flag_set(s_set->done_flag);
  if(o.cost > 0){ c.gold -= o.cost; snprintf(s_extra, sizeof(s_extra), trc("-%d or"), o.cost); }
  if(o.gold > 0){ c.gold += o.gold; snprintf(s_extra, sizeof(s_extra), trc("+%d or"), o.gold); audio::sfx_coin(); }
  if(o.item){ int it = item_index(o.item); if(it >= 0 && c.inv.add(it, o.qty)) item_ui::pickup_push(it, o.qty); }
  s_text = o.reply; s_accept = true; s_reply = true;
  if(!s_extra[0]) audio::sfx_confirm();
  return RUNNING;
}

static const gb::Color ATT_COL[4] = { gb::rgb(225,95,80), gb::rgb(120,205,110), gb::rgb(190,130,230), gb::rgb(110,170,235) };

void render(const char* npcName){
  if(!s_set) return;
  gb::fill_rect(8,150,304,84,gb::rgb(20,16,10));
  gb::fill_rect(8,150,304,2,gb::rgb(200,170,90)); gb::fill_rect(8,232,304,2,gb::rgb(200,170,90));
  ui::gold(16,153,trc(npcName),true);
  if(s_reply){
    char L[3][64]; int n = item_ui::wrap_text(hero_text(trc(s_text)), 288, L, 3);
    for(int i = 0; i < n; ++i) ui::text(16,172 + i*15,L[i],gb::rgb(235,225,190));
    if(s_accept){
      const Option& o = s_set->opt[s_sel];
      char a[48]; snprintf(a,sizeof(a),trc("%s +1 · Pouvoir %d"),trc(ATT_NAME[o.att]),player().PO);
      ui::text(16,216,a,ATT_COL[o.att]);
      if(s_extra[0]) ui::text(300 - ui::text_w(s_extra),216,s_extra,gb::rgb(245,215,95));
    }
    return;
  }
  // Attitude de la ligne selectionnee, en haut a droite
  const char* an = trc(ATT_NAME[s_set->opt[s_sel].att]);
  ui::text(300 - ui::text_w(an),153,an,ATT_COL[s_set->opt[s_sel].att]);
  for(int k = 0; k < 4; ++k){
    const Option& o = s_set->opt[k]; int y = 172 + k*15;
    if(k == s_sel) gb::fill_rect(12,y - 1,296,15,gb::rgb(60,45,20));
    gb::fill_rect(16,y + 4,6,6,ATT_COL[o.att]);                 // pastille de l'attitude
    ui::text(28,y,hero_text(trc(o.text)),k == s_sel ? gb::rgb(255,240,190) : gb::rgb(200,190,160));
  }
}

}}
