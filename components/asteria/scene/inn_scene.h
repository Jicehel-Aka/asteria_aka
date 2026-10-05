#pragma once
#include "scene/scene.h"
namespace asteria { struct InnScene : Scene { int sel=0; char msg[72]; void enter() override; void update(SceneManager&) override; void render() override; }; InnScene& inn_scene(); }
