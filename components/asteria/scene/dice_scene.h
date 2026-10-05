#pragma once
#include "scene/scene.h"
namespace asteria { struct DiceScene : Scene { int betSel=0, phase=0, pr=0, hr=0; char msg[64]; void enter() override; void update(SceneManager&) override; void render() override; }; DiceScene& dice_scene(); }
