#include "lost_levels/engine.h"
#include "lost_levels/graphics.h"

#pragma clang diagnostic ignored "-Wswitch"

using namespace std;
using namespace lain;
using namespace lost_levels;

const Size<int> LOGICAL_SIZE = Size<int>(480, 270);
const Size<int> BLOCK_SIZE = Size<int>(16, 16);

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
   InitialState(Engine& engine) : State(engine) {}

   void initialize() override {
      diagTimer = create_sdl_timer(5000);
      diagTimer->start();

      bus.subscribe("NewBlock", [this](const Event&) {
         new_block();
      });

      bus.subscribe("NewBlockTimes10", [this](const Event&) {
         new_block(100);
      });

      bus.subscribe("RemoveBlock", [this](const Event&) {
         remove_block();
      });

      bus.subscribe("RemoveBlockTimes10", [this](const Event&) {
         remove_block(100);
      });
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
            engine.get_resource_manager()->
            get_animation("question-block")->
            copy()));
      }
   }

   void update() override {
      if (diagTimer->update()) {
         Uint32 graphicsFrames = engine.get_graphics_frames();
         double fps = (double)(graphicsFrames - prevGraphicsFrames) / 5.0;
         prevGraphicsFrames = graphicsFrames;
         tfm::format(cout, "FPS: %f, Sprites: %d\n", fps, blocks.size());
      }

      for (auto b : blocks) {
         b->update();
      }
   }

   void paint() override {
      shared_ptr<Renderer> renderer = engine.get_renderer();
      engine.get_renderer()->clear();

      for (auto b : blocks) {
         renderer->render(b->get_animation(), b->get_location().round());
      }
   }

private:
   shared_ptr<Timer<Uint32>> diagTimer;
   vector<shared_ptr<Block>> blocks;
   Uint32 prevGraphicsFrames = 0;
};

class DemoInputSource : public InputSource {
   void feed(EventBus& bus) override {
      SDL_Event e;

      while (SDL_PollEvent(&e)) {
         switch(e.type) {
         case SDL_QUIT:
            bus.publish("Engine::Quit");
            break;

         case SDL_KEYDOWN:
            switch (e.key.keysym.scancode) {
            case SDL_SCANCODE_C:
               if (e.key.keysym.mod & KMOD_SHIFT) {
                  bus.publish("NewBlockTimes10");

               } else {
                  bus.publish("NewBlock");
               }
               break;

            case SDL_SCANCODE_X:
               if (e.key.keysym.mod & KMOD_SHIFT) {
                  bus.publish("RemoveBlockTimes10");

               } else {
                  bus.publish("RemoveBlock");
               }
               break;

            case SDL_SCANCODE_Q:
               bus.publish("Engine::Quit");
               break;
            }
         }
      }
   }
};

class DemoEngine : public Engine {
public:
   void initialize() override {
      shared_ptr<Window> window = Window::Builder()
         .with_title("Flashing Question Blocks Demo")
         .with_size(Size<int>(960, 540))
         .create();

      shared_ptr<Renderer> renderer = Renderer::Builder(window)
         .with_logical_size(LOGICAL_SIZE)
         .with_acceleration()
         .with_vsync()
         .create();

      renderer->set_draw_color(Color(0, 0, 0));
      set_renderer(renderer);

      cout << "Called DemoEngine::initialize()." << endl;
      set_input_source(make_shared<DemoInputSource>());
      get_resource_manager()->load_file("simple/resource.json");

      push_state<InitialState>();
   }
};

int main(int argc, char** argv) {
   DemoEngine engine;
   srand(time(0));

   return engine.run(argc, argv);
}

