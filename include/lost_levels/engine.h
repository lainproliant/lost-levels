/*
 * engine: The skeleton for a game engine.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Sunday, Jan 25 2015
 */
#pragma once
#include <csignal>

#include "common.h"
#include "lost_levels/timer.h"
#include "lost_levels/event.h"
#include "lost_levels/input.h"
#include "lost_levels/graphics.h"
#include "lost_levels/resources.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;
   using namespace lain::getopt;

   class Engine;

   class State {
   public:
      State(Engine& engine) : engine(engine)
      { }

      virtual ~State() { }

      virtual void initialize() = 0;
      virtual void update() = 0;
      virtual void paint() = 0;

      EventBus& get_event_bus() {
         return bus;
      }

      void process_events() {
         bus.process_events();
      }

   protected:
      EventBus bus;
      Engine& engine;
      shared_ptr<ResourceManager> resourceManager = nullptr;
   };

   class Engine {
   public:
      Engine() : parser(OptionParser("", {})) { }

      virtual ~Engine() { }

      Uint32 get_physics_frames() const {
         return physicsTimer->get_frames();
      }

      Uint32 get_graphics_frames() const {
         return graphicsTimer->get_frames();
      }

      shared_ptr<Renderer> get_renderer() const {
         return renderer;
      }

      shared_ptr<ResourceManager> get_resource_manager() const {
         return resourceManager;
      }

      int run(int argc, char** argv) {
         signal(SIGSEGV, signal_callback);

         try {
            parser.parse(argc, argv);

            phase1();
            phase2();
            phase3();

            graphicsTimer->start();
            physicsTimer->start();

            if (! inputSource) {
               throw Exception("No input source is defined.");
            }

            /*
            if (! audioMixer) {
               throw Exception("No audio mixer was defined.");
            }
            */

            if (states.empty()) {
               throw Exception("No initial state was defined.");
            }

            while (! states.empty()) {
               input();
               update();
               paint();

               SDL_Delay(graphicsTimer->get_wait_time());
            }

            return 0;

         } catch (const exception& e) {
            tfm::format(cerr, "FATAL: (%s): %s\nAborted.\n",
               typeid(e).name(), string(e.what()));

            return 1;
         }
      }

      void pop_state() {
         states.pop();

         if (! states.empty()) {
            currentState = states.top();
         }
      }

      template<class T>
      void push_state() {
         currentState = make_shared<T>(*this);
         currentState->initialize();
         states.push(currentState);
      }

   protected:
      const OptionParser& get_options() const {
         return parser;
      }

      void set_graphics_timer(shared_ptr<Timer<Uint32>> timer) {
         graphicsTimer = timer;
      }

      void set_input_source(shared_ptr<InputSource> inputSource) {
         this->inputSource = inputSource;
      }

      void set_option_parser(const OptionParser& parser) {
         this->parser = parser;
      }

      void set_physics_timer(shared_ptr<Timer<Uint32>> timer) {
         physicsTimer = timer;
      }

      void set_renderer(shared_ptr<Renderer> renderer) {
         this->renderer = renderer;

         set_resource_manager(make_shared<ResourceManager>(
            physicsTimer, get_renderer()));
      }

      void set_resource_manager(shared_ptr<ResourceManager> resourceManager) {
         this->resourceManager = resourceManager;
      }

      /*
      void set_audio_mixer(shared_ptr<AudioMixer> audioMixer) {
         this->audioMixer = audioMixer;
      }
      */

      virtual void phase1() {
         if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw Exception(string("Failed to initialize SDL: ") +
               SDL_GetError());
         }

         set_physics_timer(create_sdl_timer(1000 / 100, true));
         set_graphics_timer(create_sdl_timer(1000 / 60));
      }

      virtual void phase2() {
         initialize();
      }

      virtual void phase3() {
         bus.subscribe("Engine::Quit", [this](const Event&) {
            while (! this->states.empty()) {
               this->states.pop();
            }
         });
      }

      virtual void initialize() { }

      virtual void on_fatal_exception(const exception&) { }

      void input() {
         inputSource->feed(bus);
         bus.channel(currentState->get_event_bus());
      }

      bool update() {
         bus.process_events();

         while (physicsTimer->update()) {
            currentState->process_events();
            currentState->update();
         }

         return true;
      }

      void paint() {
         if (graphicsTimer->update()) {
            currentState->paint();
            renderer->display();
         }
      }

   private:
      static void signal_callback(int signal) {
         tfm::format(cerr, "FATAL: Caught signal %d (%s): %s\nAborted.\n",
            signal, strsignal(signal),
            format_stacktrace(generate_stacktrace()));
         exit(1);
      }

      shared_ptr<Timer<Uint32>> physicsTimer = nullptr;
      shared_ptr<Timer<Uint32>> graphicsTimer = nullptr;
      shared_ptr<InputSource> inputSource = nullptr;
      shared_ptr<Renderer> renderer = nullptr;
      shared_ptr<ResourceManager> resourceManager = nullptr;

      stack<shared_ptr<State>> states;
      shared_ptr<State> currentState = nullptr;

      EventBus bus;

      OptionParser parser;
      int returnCode = 0;
   };
}
