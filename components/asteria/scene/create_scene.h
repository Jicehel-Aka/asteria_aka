#pragma once
#include "scene/scene.h"
namespace asteria {
struct CreateScene : Scene {
  int step=0, selChoice=0, cur=0, st[4]={8,8,8,8}, pool=18;
  // DELTA-02 : identite du voyageur cree (etape 3) : nom (12 car. max) + sexe
  char nm[13]="Aren        "; int ncur=0; bool editing=false; bool female=false; int idSel=0;
  void enter() override; void update(SceneManager&) override; void render() override;
};
CreateScene& create_scene();
}
