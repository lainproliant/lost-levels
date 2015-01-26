#include "lost_levels/timer.h"
#include "lain/testing.h"

using namespace std;
using namespace lain;
using namespace lain::testing;
using namespace lost_levels;

double run_timer_sim(int fps, const int frames) {
   shared_ptr<Timer<Uint32>> timer = create_sdl_timer(1000 / fps);
   timer->start();

   double totalErrorTime = 0.0;
   Uint32 terr;

   while (timer->get_frames() < frames) {
      if (timer->update(&terr)) {
         cout << ".";
         cout.flush();

         totalErrorTime += terr;

      } else {
         SDL_Delay(5 + rand() % 5);
         SDL_Delay(timer->get_wait_time());
      }
   }

   cout << endl;

   return totalErrorTime;
}


int main(int argc, char** argv) {
   srand(time(0));

   return TestSuite("lost_levels timer tests")
      .die_on_signal(SIGSEGV)
      .test("Timer-001: Testing average error rate at various FPS", [&]()->bool {
         for (int fps = 15; fps <= 120; fps *= 2) {
            cout << "Testing " << fps << "fps:" << endl;
            double totalErrorTime = run_timer_sim(fps, 100);
            double avgErrorPerFrame = totalErrorTime / 100;
            assert_true(avgErrorPerFrame < 1.0);
         }

         return true;
      })
      .test("Timer-002: Testing example physics and graphics timers", [&]()->bool {
         const int SECONDS = 15;
         const int GRAPHICS_FPS = 30;
         const int PHYSICS_FPS = 100;

         auto graphics_timer = create_sdl_timer(1000 / GRAPHICS_FPS);
         auto physics_timer = create_sdl_timer(1000 / PHYSICS_FPS, true); // accumulate

         graphics_timer->start();
         physics_timer->start();

         Uint32 startTime = SDL_GetTicks();
         double totalRenderTime = 0.0;
         double totalRenderErrorTime = 0.0;
         double totalPhysicsErrorTime = 0.0;

         while (SDL_GetTicks() - startTime < (1000 * SECONDS)) {
            Uint32 terr;
            int physics_frames = 0;

            if (graphics_timer->update(&terr)) {
               Uint32 renderStart = SDL_GetTicks();
               cout << ":";
               SDL_Delay(5 + rand() % 5);
               cout.flush();
               Uint32 renderEnd = SDL_GetTicks();
               totalRenderTime += renderEnd - renderStart;
               totalRenderErrorTime += terr;
            }

            while (physics_timer->update(&terr)) {
               cout << ".";
               cout.flush();
               totalPhysicsErrorTime += terr;
               SDL_Delay(rand() % 5);
            }

            SDL_Delay(graphics_timer->get_wait_time());
         }

         double avgRenderErrorTime = totalRenderErrorTime / graphics_timer->get_frames();
         double avgPhysicsErrorTime = totalPhysicsErrorTime / physics_timer->get_frames();
         double avgRenderTime = totalRenderTime / graphics_timer->get_frames();
         double correctedAvgPhysicsErrorTime = avgPhysicsErrorTime - avgRenderTime;

         cout << endl;
         cout << "Total graphics frames: "
              << graphics_timer->get_frames() << endl
              << "Total physics frames: "
              << physics_timer->get_frames() << endl
              << "Avg render time: "
              << totalRenderTime / graphics_timer->get_frames() << endl
              << "Avg physics error time - avg render time: "
              << correctedAvgPhysicsErrorTime << endl
              << "Avg render error time: "
              << avgRenderErrorTime << endl;

         assert_true(avgRenderErrorTime < 1.0);
         assert_true(correctedAvgPhysicsErrorTime < 1.0);
         return true;
      })
      .run();
}

