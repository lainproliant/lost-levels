/*
 * engine: The skeleton for a game engine.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Sunday, Jan 25 2015
 */
#pragma once
#include <csignal>
#include <stack>

#include "lain/exception.h"
#include "lost_levels/timer.h"
#include "lost_levels/event.h"
#include "lost_levels/input.h"
#include "lost_levels/graphics.h"
#include "lost_levels/resources.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   class Engine;

   class EngineException : public Exception {
      using Exception::Exception;
   };

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
   };

   class Engine {
   public:
      Engine() {
         bus.subscribe("Engine.Quit", [this](const Event&) {
            clear_state();
         });
      }

      virtual ~Engine() { }

      shared_ptr<Window> get_window() const {
         return window;
      }

      shared_ptr<Renderer> get_renderer() const {
         return renderer;
      }

      shared_ptr<Timer<uint32_t>> get_graphics_timer() {
         return graphicsTimer;
      }

      shared_ptr<Timer<uint32_t>> get_physics_timer() {
         return physicsTimer;
      }

      int run() {
         signal(SIGSEGV, signal_callback);

         try {
            initialize();

            require(graphicsTimer != nullptr, "graphics timer");
            require(physicsTimer != nullptr, "physics timer");
            require(window != nullptr, "window");
            require(renderer != nullptr, "renderer");
            require(inputProvider != nullptr, "input provider");
            require(! states.empty(), "initial state");

            graphicsTimer->start();
            physicsTimer->start();

            while (! states.empty()) {
               input();
               update();
               paint();
               delay();
            }

            return 0;

         } catch (const exception& e) {
            tfm::format(cerr, "FATAL: (%s): %s\nAborted.\n",
               typeid(e).name(), string(e.what()));

            return 1;
         }
      }

      template<class T, class... Args>
      void push_state(Args&&... args) {
         currentState = make_shared<T>(*this, args...);
         currentState->initialize();
         states.push(currentState);
      }

      void pop_state() {
         states.pop();

         if (! states.empty()) {
            currentState = states.top();
         }
      }

      template<class T, class... Args>
      void reset_state(Args&&... args) {
         clear_state();
         push_state<T>(args...);
      }

      void clear_state() {
         while (! states.empty()) {
            states.pop();
         }
      }

   protected:
      void set_window(shared_ptr<Window> window) {
         this->window = window;
      }

      void set_renderer(shared_ptr<Renderer> renderer) {
         this->renderer = renderer;
      }

      void set_graphics_timer(shared_ptr<Timer<uint32_t>> timer) {
         graphicsTimer = timer;
      }

      void set_physics_timer(shared_ptr<Timer<uint32_t>> timer) {
         physicsTimer = timer;
      }

      void set_input_provider(shared_ptr<InputProvider> provider) {
         inputProvider = provider;
      }

      virtual void initialize() = 0;

      virtual void input() {
         inputProvider->channel(bus);
         bus.channel(currentState->get_event_bus());
      }

      virtual bool update() {
         bus.process_events();

         while (physicsTimer->update()) {
            currentState->process_events();
            currentState->update();
         }

         return true;
      }

      virtual void paint() {
         if (graphicsTimer->update()) {
            currentState->paint();
            renderer->display();
         }
      }

      virtual void delay() { }

      void require(bool expr, const string& item) {
         if (! expr) {
            throw EngineException(tfm::format(
               "No %s was provided.", item));
         }
      }

   private:
      static void signal_callback(int signal) {
         tfm::format(cerr, "FATAL: Caught signal %d (%s): %s\nAborted.\n",
            signal, strsignal(signal),
            format_stacktrace(generate_stacktrace()));
         exit(1);
      }

      shared_ptr<Timer<uint32_t>> physicsTimer = nullptr;
      shared_ptr<Timer<uint32_t>> graphicsTimer = nullptr;

      shared_ptr<InputProvider> inputProvider = nullptr;
      shared_ptr<Window> window = nullptr;
      shared_ptr<Renderer> renderer = nullptr;

      stack<shared_ptr<State>> states;
      shared_ptr<State> currentState = nullptr;

      EventBus bus;
      int returnCode = 0;
   };
}
