#include "lost_levels/engine.h"
#include "lost_levels/graphics_sdl2.h"
#include "lost_levels/timer_sdl2.h"

#pragma clang diagnostic ignored "-Wswitch"

using namespace std;
using namespace lain;
using namespace lost_levels;

const Size<int> WINDOW_SIZE = Size<int>(960, 540);
const Size<int> LOGICAL_SIZE = Size<int>(480, 270);
const Size<int> BLOCK_SIZE = Size<int>(16, 16);
const Color CLEAR_COLOR = Color(0, 0, 0);

class Block {
public:
   Block(Point<float> location,
         Vector<float> velocity,
         shared_ptr<Animation> animation) :
            animation(animation), location(location),
            velocity(velocity) {
      animation->start();
   }

   const Point<float>& get_location() const {
      return location;
   }

   shared_ptr<const Animation> get_animation() const {
      return animation;
   }

   void update() {
      location += velocity;

      if (location.x < 0 || location.x + BLOCK_SIZE.width > LOGICAL_SIZE.width) {
         velocity.vx = -velocity.vx;
      }

      if (location.y < 0 || location.y + BLOCK_SIZE.height > LOGICAL_SIZE.height) {
         velocity.vy = -velocity.vy;
      }

      animation->update();
   }

private:
   shared_ptr<Animation> animation;
   Point<float> location;
   Vector<float> velocity;
};

class InitialState : public State {
public:
   InitialState(Engine& engine, const ResourceManager& rm) : State(engine), rm(rm) { }
   void initialize() override {
      diagTimer = sdl2::create_timer(5000);
      diagTimer->start();
   }

   void remove_block(int num = 1) {
      for (int x = 0; x < num && ! blocks.empty(); x++) {
         blocks.pop_back();
      }
   }

   void new_block(int num = 1) {
      for (int x = 0; x < num; x++) {
         Vector<float> velocity = Vector<float>(
            (float)(rand() % 100) / 100.0,
            (float)(rand() % 100) / 100.0);
         Point<float> location = Point<float>();

         blocks.push_back(make_shared<Block>(
            location, velocity,
            rm.get_animation("question-block")->copy()));
      }
   }

   void input() override {
      SDL_Event e;

      while (SDL_PollEvent(&e)) {
         switch(e.type) {
         case SDL_QUIT:
            engine.pop_state();
            break;

         case SDL_KEYDOWN:
            switch (e.key.keysym.scancode) {
            case SDL_SCANCODE_C:
               if (e.key.keysym.mod & KMOD_SHIFT) {
                  new_block(100);

               } else {
                  new_block();
               }

               break;

            case SDL_SCANCODE_X:
               if (e.key.keysym.mod & KMOD_SHIFT) {
                  remove_block(100);

               } else {
                  remove_block();
               }

               break;

            case SDL_SCANCODE_Q:
               engine.pop_state();
               break;
            }

            break;
         }
      }
   }

   void update() override {
      if (diagTimer->update()) {
         uint32_t graphicsFrames = engine.get_graphics_timer()->get_frames();
         double fps = (double)(graphicsFrames - prevGraphicsFrames) / 5.0;
         prevGraphicsFrames = graphicsFrames;
         tfm::format(cout, "FPS: %f, Sprites: %d\n", fps, blocks.size());
      }

      for (auto b : blocks) {
         b->update();
      }
   }

   void paint() override {
      auto renderer = engine.get_renderer();

      for (auto b : blocks) {
         renderer->render(b->get_animation(), b->get_location().round());
      }
   }

private:
   shared_ptr<Timer<uint32_t>> diagTimer;
   vector<shared_ptr<Block>> blocks;
   uint32_t prevGraphicsFrames = 0;

   const ResourceManager& rm;
};

class DemoEngine : public Engine {
public:
   DemoEngine() : Engine() { }

   void initialize() override {
      set_window(sdl2::create_window(Size<int>(1280, 720)));
      set_renderer(sdl2::create_renderer(get_window()));
      set_physics_timer(sdl2::create_timer(1000 / 100, true));
      set_graphics_timer(sdl2::create_timer(1000 / 60));

      get_renderer()->set_logical_size(LOGICAL_SIZE);
      get_renderer()->set_draw_color(CLEAR_COLOR);

      rm = make_shared<ResourceManager>(
            get_physics_timer(), get_renderer());
      rm->load_file("simple/resource.json");

      push_state<InitialState>(*rm);
   }

   void delay() override {
      SDL_Delay(get_graphics_timer()->get_wait_time());
   }

private:
   shared_ptr<ResourceManager> rm;
};

int main() {
   DemoEngine engine;
   srand(time(0));

   return engine.run();
}

