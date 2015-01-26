#include <cstdio>
#include <cstdlib>
#include <SDL2/SDL.h>

#include "config.h"

using namespace std;
using namespace lost_levels;

static const char* gs_appName = APP_NAME " " APP_VERSION;

int main(int argc, char** argv)
{
   SDL_Window* window;
   SDL_Renderer* renderer;

   if (SDL_Init(SDL_INIT_FLAGS) != 0) {
      fprintf(stderr, "Failed to initialize SDL: %s\n",
              SDL_GetError());
      exit(1);
   }

   atexit(SDL_Quit);

   window = SDL_CreateWindow(gs_appName,
         SDL_WINDOWPOS_CENTERED,
         SDL_WINDOWPOS_CENTERED,
         SDL_WINDOW_DEFAULT_WIDTH,
         SDL_WINDOW_DEFAULT_HEIGHT,
         SDL_WINDOW_FLAGS);

   if (window == NULL) {
      fprintf(stderr, "Failed to create SDL_Window: %s\n",
              SDL_GetError());
      exit(1);
   }

   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_FLAGS);

   if (renderer == NULL) {
      fprintf(stderr, "Failed to create SDL_Renderer: %s\n",
              SDL_GetError());
   }
}
