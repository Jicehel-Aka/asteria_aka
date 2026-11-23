#pragma once
#include "scene/scene.h"
namespace asteria { struct ForgeScene : Scene { void enter() override; void update(SceneManager&) override; void render() override; }; ForgeScene& forge_scene(); }
