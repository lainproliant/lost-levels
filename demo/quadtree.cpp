#include "lost_levels/diag.h"
#include "lost_levels/engine.h"
#include "lost_levels/graphics_sdl2.h"
#include "lost_levels/timer_sdl2.h"

#include <set>

#pragma clang diagnostic ignored "-Wswitch"

using namespace std;
using namespace lain;
using namespace lost_levels;

const Size<int> WINDOW_SIZE = Size<int>(640, 360);
const Size<int> LOGICAL_SIZE = Size<int>(256, 224);
const Size<float> BLOCK_SIZE = Size<float>(16, 16);
const Color CLEAR_COLOR = Color(0, 0, 0);
const Color COLL_TREE_COLOR = Color(255, 0, 0);
const Color BKG_RECT_COLOR = Color(0, 255, 255);
const int MAX_BLOCKS = 256;

const Rect<float> LEVEL_RECT = Rect<float>(0, 0,
      LOGICAL_SIZE.width,
      LOGICAL_SIZE.height);

class Block {
public:
   Block(int id, Point<float> location, Vector<float> velocity,
         shared_ptr<Animation> animation) :
      id(id), rect(Rect<float>(location, BLOCK_SIZE)),
      velocity(velocity), animation(animation)
   {
      prevRect = rect;
      animation->start();
   }

   const Rect<float>& get_rect() const {
      return rect;
   }
   
   shared_ptr<const Animation> get_animation() const {
      return animation;
   }

   void update() {
      prevRect = rect;
      rect = rect.translate(velocity);
      
      if (rect.pt.x < 0 || rect.pt.x + rect.sz.width > LEVEL_RECT.sz.width) {
         bounce_x();
      }

      if (rect.pt.y < 0 || rect.pt.y + rect.sz.height > LEVEL_RECT.sz.height) {
         bounce_y();
      }

      animation->update();
   }

   void bounce_x() {
      velocity.vx *= -1;
   }

   void bounce_y() {
      velocity.vy *= -1;
   }

   int get_id() const {
      return id;
   }
   
private:
   int id;
   Rect<float> rect;
   Rect<float> prevRect;
   Vector<float> velocity;
   shared_ptr<Animation> animation;
};

class InitialState : public State {
public:
   InitialState(Engine& engine, const ResourceManager& rm) :
      State(engine), rm(rm), backgroundVelocity(Vector<float>(0.25, 0)),
      frameCalculator(sdl2::create_frame_calculator(engine.get_graphics_timer())),
      collTree(LEVEL_RECT) { }

   void initialize() override {
      diagTimer = sdl2::create_timer(5000);
      diagTimer->start();

      statusFont = ImageFont::create(rm.get<Image>("font"), Size<int>(7, 8));
      statusFont->set_start_char('!');
      background = rm.get<Animation>("stars");
      background->start();
   }

   void remove_block(int num = 1) {
      for (int x = 0; x < num && ! blocks.empty(); x++) {
         blocks.pop_back();
      }
   }

   void new_block(int num = 1) {
      for (int x = 0; x < num; x++) {
         if (blocks.size() >= MAX_BLOCKS) {
            return;
         }

         Vector<float> velocity = Vector<float>(
            (float)(rand() % 100) / 100.0,
            (float)(rand() % 100) / 100.0);
         Point<float> location = Point<float>();
         
         blocks.push_back(make_shared<Block>(blocks.size(),
            location, velocity,
            rm.get<Animation>("question-block")->copy()));
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

            case SDL_SCANCODE_H:
               backgroundVelocity -= Vector<float>(0.25, 0.00);
               break;

            case SDL_SCANCODE_L:
               backgroundVelocity += Vector<float>(0.25, 0.00);
               break;

            case SDL_SCANCODE_J:
               backgroundVelocity -= Vector<float>(0.00, 0.25);
               break;

            case SDL_SCANCODE_K:
               backgroundVelocity += Vector<float>(0.00, 0.25);
               break;

            case SDL_SCANCODE_P:
               paused = !paused;
               break;
            }

            break;
         }
      }
   }
   
   void update() override {
      bool diagPrint = false;
      collTree.clear();
      collTree.debug_split();
      set<pair<int, int>> collisionPairs;

      for (auto b : blocks) {
         if (!paused) {
            b->update();
         }

         collTree.insert(b, b->get_rect());
      }
      
      if (diagTimer->update()) {
         uint32_t graphicsFrames = engine.get_graphics_timer()->get_frames();
         double fps = (double)(graphicsFrames - prevGraphicsFrames) / 5.0;
         prevGraphicsFrames = graphicsFrames;
         tfm::format(cout, "FPS: %f, Sprites: %d\n", fps, blocks.size());
         tfm::format(cout, "backgroundVelocity = %s\n", backgroundVelocity);
         tfm::format(cout, "backgroundPosition = %s\n", backgroundPosition);
         diagPrint = true;
      }

      background->update();
      backgroundPosition += backgroundVelocity;

      frameCalculator->update();
   }

   void paint() override {
      auto renderer = engine.get_renderer();
      
      Rect<int> bkgRect = Rect<int>(Point<int>(), Size<int>(LOGICAL_SIZE.width,
               background->get_size().height));
      renderer->render_pattern(background, backgroundPosition.round(), bkgRect);

      for (auto b : blocks) {
         renderer->render(b->get_animation(), b->get_rect().pt.round());
         renderer->print_string(b->get_rect().pt.round(), statusFont, tfm::format("%x", b->get_id()));
      }

      render_coll_tree(collTree, COLL_TREE_COLOR);
      renderer->set_draw_color(BKG_RECT_COLOR);
      renderer->draw_rect(bkgRect);
      renderer->set_draw_color(CLEAR_COLOR);

      renderer->print_string(Point<int>(0, LOGICAL_SIZE.height - statusFont->get_size().height * 2),
                             statusFont,
                             tfm::format("Sprites: %d\nFPS: %d", blocks.size(), frameCalculator->get_fps()));

   }

   void render_coll_tree(const CollisionTree<float, shared_ptr<Block>>& collTree, const Color& color) {
      auto renderer = engine.get_renderer();

      renderer->set_draw_color(color);
      renderer->draw_rect(collTree.get_rect().round());

      for (auto quad : collTree.get_quadrants()) {
         render_coll_tree(*quad, color);
      }
   }

private:
   pair<int, int> ordered_id_pair(int idA, int idB) {
      return pair<int, int>(
            idA < idB ? idA : idB,
            idA < idB ? idB : idA);
   }

   shared_ptr<Font> statusFont;
   shared_ptr<Animation> background;
   Point<float> backgroundPosition;
   shared_ptr<Timer<uint32_t>> diagTimer;
   vector<shared_ptr<Block>> blocks;
   uint32_t prevGraphicsFrames = 0;
   bool paused = false;

   const ResourceManager& rm;
   Vector<float> backgroundVelocity;
   shared_ptr<FrameCalculator<uint32_t>> frameCalculator;
   CollisionTree<float, shared_ptr<Block>> collTree;
};

class DemoEngine : public Engine {
public:
   DemoEngine() : Engine() { }

   void initialize() override {
      set_window(sdl2::create_window(Size<int>(1280, 720),
                 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));
      set_renderer(sdl2::create_renderer(get_window()));
      set_physics_timer(sdl2::create_timer(1000 / 100, true));
      set_graphics_timer(sdl2::create_timer(1000 / 60));

      get_renderer()->set_logical_size(LOGICAL_SIZE);
      get_renderer()->set_draw_color(CLEAR_COLOR);

      rm = make_shared<ResourceManager>(get_physics_timer(),
            sdl2::create_image_loader(get_renderer()));
      rm->load_file("simple-rc/resource.json");

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

