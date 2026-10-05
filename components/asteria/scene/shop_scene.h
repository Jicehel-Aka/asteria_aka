#pragma once
#include "scene/scene.h"
namespace asteria { struct ShopScene : Scene { void enter() override; void update(SceneManager&) override; void render() override; }; ShopScene& shop_scene(); }
