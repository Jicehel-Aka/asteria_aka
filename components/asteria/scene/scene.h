#pragma once
namespace asteria {
enum class SceneId { TITLE, CREATE, RULES, WORLD, COMBAT, SHOP, QUIT };
class SceneManager;
struct Scene { virtual void enter(){} virtual void update(SceneManager&)=0; virtual void render()=0; virtual ~Scene(){} };
class SceneManager {
public:
  void set(SceneId id);
  void update(){ if(s_) s_->update(*this); }
  void render(){ if(s_) s_->render(); }
  SceneId current() const { return cur_; }
private:
  SceneId cur_=SceneId::TITLE; Scene* s_=nullptr;
};
}
