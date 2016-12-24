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
#include "lain/util.h"
#include "lost_levels/timer.h"
#include "lost_levels/graphics.h"
#include "lost_levels/resources.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   class Engine;

   class EngineException : public Exception {
      using Exception::Exception;
   };

   /**
    * State <abstract class>
    *
    * Represents a game state.  Game states are pushed and poped from
    * the Engine, which acts as a context and state stack.
    *
    * States are closely tied to the engine into which they are inserted,
    * and are capable of mutating state by pushing or poping other states.
    *
    * Other than top level diagnostic code, control is delegated to the
    * currently active state for input(), update(), and paint() events
    * in the game loop.
    *
    * USAGE:
    * - Extend this class to implement your game states.
    * - Push your initial state in the initialize() method of your
    *   Engine implementation.
    *
    *   push_state<MyState>(param1, param2, param3);
    *
    * - Push other states from within this state, and pop them to end
    *   the current state.  Emptying the engine's state stack will end
    *   the program.
    *
    *   - In MyState:
    *       engine.push_state<OtherState>(param1);
    *
    *   - In OtherState:
    *       engine.pop_state(); // Return to MyState
    */
   class State {
   public:
      State(Engine& engine) : engine(engine)
      { }

      virtual ~State() { }

      /**
       * Invoked once after the state is constructed before any of
       * the input(), update(), or paint() methods are invoked.
       */
      virtual void initialize() = 0;

      /**
       * Called by the Engine to prompt a check for input.  There may
       * or may not be input queued.
       */
      virtual void input() = 0;

      /**
       * Called when the Engine physics update timer fires.  Use this to
       * update internal state and objects in the game.
       */
      virtual void update() = 0;

      /**
       * Called when the graphics update timer fires.  Use this to draw
       * the scene.
       */
      virtual void paint() = 0;

   protected:
      Engine& engine;
   };

   /**
    * Engine <abstract class>
    *
    * An abstract class encapsulating most of the behavior for the
    * lower level management of state and the game loop.
    *
    * USAGE:
    * - You MUST extend this class and implement the initialize() method.
    *   In this method you should do the following things:
    *    - Create a window, then call set_window().
    *
    *       set_window(sdl2::create_window(Size<int>(1920, 1080));
    *
    *    - Create a renderer, then call set_renderer().
    *
    *       set_renderer(sdl2::create_renderer(get_window());
    *
    *    - Set physics and graphics timers.
    *
    *       // 100FPS no frame skip.
    *       set_physics_timer(sdl2::create_timer(1000 / 10), true);
    *
    *       // 60FPS frame skip (with recovery)
    *       set_graphics_timer(sdl2::create_timer(1000 / 60));
    *
    *    - Optionally initialize any other settings, such as the
    *      renderer's logical size, default draw color, or load data
    *      into a ResourceManager instance.
    *
    *    - Push your initial state onto the stack.  This is performed via
    *      a template function push_state<T>().  Put the name of your
    *      state class in place of T, and pass any constructor parameters
    *      for the class to push_state.  The first parameter to your
    *      State class constructor must be a non-const reference to
    *      Engine.
    *
    *      class AnimalState : public State {
    *         AnimalState(Engine& engine, const string& animalName) ...
    *      };
    *  
    *      push_state<AnimalState>("Puppy");
    */
   class Engine {
   public:
      Engine() { }
      virtual ~Engine() { }

      shared_ptr<Window> get_window() const {
         return window;
      }

      shared_ptr<Renderer> get_renderer() const {
         return renderer;
      }

      shared_ptr<Timer<uint32_t>> get_graphics_timer() const {
         return graphicsTimer;
      }

      shared_ptr<Timer<uint32_t>> get_physics_timer() const {
         return physicsTimer;
      }

      /**
       * This method should be called by your main() function to start
       * the game loop and run your game.  It calls your initialize()
       * function, then proceeds into the game loop until the state
       * stack is empty.
       *
       * NOTES:
       * - This method should only be called once, and only one
       *   Engine object should be instantiated and used.
       * - Upon calling this method, a static method will be registered
       *   to handle SIGSEGV and will attempt to print a stack trace
       *   when it is caught, then it will terminate the program with
       *   exit code 1.  Stack traces can only be enabled if your
       *   compiler supports generating them, all symbols are compiled
       *   in debug mode (-g), and the following compile-time variables
       *   are defined:
       *
       *       -DLAIN_ENABLE_STACKTRACE
       *       -DLAIN_STACKTRACE_IN_DESCRIPTION
       *
       *   This behavior is defined by the "lain/exception.h" file in the
       *   toolbox project, which is a submodule of this library.
       *
       * @return 0 if the game exited normally, or 1 if an exception
       *    was caught at the top level and the game was aborted.
       */
      int run() {
         signal(SIGSEGV, signal_callback);

         try {
            initialize();

            util::require(graphicsTimer, "Graphics timer is required, call set_physics_timer() in your initialize() method");
            util::require(physicsTimer, "Physics timer is required, call set_physics_timer() in your initialize() method");
            util::require(window, "Window is required, call set_window() in your initialize() method");
            util::require(renderer, "Renderer is required, call set_renderer() in your initialize() method");
            util::assertTrue(! states.empty(), "Initial state is required, call put_state() in your initialize() method");

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

      /**
       * Push a new state instance onto the stack.
       *
       * @params Args... Arguments to the State constructor which
       *    follow the required non-const Engine reference.
       */
      template<class T, class... Args>
      void push_state(Args&&... args) {
         currentState = make_shared<T>(*this, args...);
         currentState->initialize();
         states.push(currentState);
      }

      /**
       * Pop the current state from the stack, replacing it with
       * the next state from the top of the stack if the stack
       * is not empty.
       *
       * If the stack is empty, the program will terminate with
       * exit code 0 on the next iteration of the game loop.
       */
      void pop_state() {
         states.pop();

         if (! states.empty()) {
            currentState = states.top();
         }
      }

      /**
       * Clear all other state objects from the stack and replace
       * them with the given state.  This can be used to reset
       * the game to its initial state.
       */
      template<class T, class... Args>
      void reset_state(Args&&... args) {
         clear_state();
         push_state<T>(args...);
      }

      /**
       * Clear all state objects from the stack.
       *
       * If no other state is pushed, the program will terminate
       * with exit code 0 on the next iteration of the game loop.
       */
      void clear_state() {
         while (! states.empty()) {
            states.pop();
         }
      }

   protected:
      /**
       * Set the window into which graphics will be drawn.
       *
       * A window is not strictly necessary, as some implementations
       * of Renderer do not require a Window (e.g., graphics_qt5.h).
       * graphics_sdl2.h requires a Window as it is used to create
       * the Renderer.
       */
      void set_window(shared_ptr<Window> window) {
         this->window = window;
      }

      /**
       * Set the renderer.  This is the primary interface to the graphics
       * pipeline and is required to be set during initialize().
       */
      void set_renderer(shared_ptr<Renderer> renderer) {
         this->renderer = renderer;
      }

      /**
       * Set the graphics timer, controlling how frequently the current
       * scene is drawn to the screen.
       *
       * This must be called during the initialize() method.
       */
      void set_graphics_timer(shared_ptr<Timer<uint32_t>> timer) {
         graphicsTimer = timer;
      }

      /**
       * Set the graphics timer, controlling how frequently and at
       * what scale the objects in the game are updated.
       *
       * This must be called during the initialize() method.
       */
      void set_physics_timer(shared_ptr<Timer<uint32_t>> timer) {
         physicsTimer = timer;
      }

      /**
       * Abstract method.  Implement this method and follow the
       * instructions outlined in the Engine class docs above.
       */
      virtual void initialize() = 0;

      /**
       * Invoked during every iteration of the game loop to allow
       * for input polling.
       *
       * By default, this method delegates to the current state's
       * input() method.
       */
      virtual void input() {
         currentState->input();
      }

      /**
       * Invoked during every iteration of the game loop to allow
       * for the update timer to be updated.
       *
       * Overriding this method is discouraged.
       */
      virtual void update() {
         while (physicsTimer->update()) {
            currentState->update();
            diag_update();
         }
      }

      /**
       * Invoked during every iteration of the game loop to allow
       * for the graphics timer to be updated.
       *
       * Overriding this method is discouraged.
       */
      virtual void paint() {
         if (graphicsTimer->update()) {
            get_renderer()->clear();
            currentState->paint();
            diag_paint();
            renderer->display();
         }
      }

      /**
       * Invoked during every update cycle.  Does nothing by default.
       *
       * Override this method to update diagnostic information to be
       * shown on the screen, e.g. a FrameCalculator to print paint FPS.
       *
       * This should ideally not be used for your game's HUD or any other
       * non-diagnostic purpose.
       */
      virtual void diag_update() { }

      /**
       * Invoked during every paint cycle after all other drawing is
       * completed.  Does nothing by default.
       *
       * Override this method to display diagnostic information to the
       * screen above all other objects.  This should ideally not be used
       * for your game's HUD or any other non-diagnostic printing.
       */
      virtual void diag_paint() { }

      /**
       * Invoked at the end of each game loop.  Does nothing by default.
       *
       * Override this method to save energy by sleeping and allowing
       * other processes to execute.  Note that by sleeping, you are
       * passing control to other processes which may affect the
       * responsiveness of your game and the refresh rate of your graphics.
       *
       * You may want to decide whether or not sleeping should be performed
       * programmatically by checking if the game is in the foreground or
       * if the device is on battery power.
       *
       * EXAMPLE:
       *
       *    void delay() override {
       *       SDL_Delay(get_graphics_timer()->get_wait_time());
       *    }
       */
      virtual void delay() { }

   private:
      static void signal_callback(int signal) {
         tfm::format(cerr, "FATAL: Caught signal %d (%s): %s\nAborted.\n",
            signal, strsignal(signal),
            format_stacktrace(generate_stacktrace()));
         exit(1);
      }

      shared_ptr<Timer<uint32_t>> physicsTimer = nullptr;
      shared_ptr<Timer<uint32_t>> graphicsTimer = nullptr;

      shared_ptr<Window> window = nullptr;
      shared_ptr<Renderer> renderer = nullptr;

      stack<shared_ptr<State>> states;
      shared_ptr<State> currentState = nullptr;

      int returnCode = 0;
   };
}
