#include "viewport.h"

using namespace std;
using namespace lost_levels;

Viewport::Viewport(int w, int h) {
   width = w;
   height = h;
}

Viewport::~Viewport() {
   for (Layer* layer : layers) {
      delete layer;
   }
}

void Viewport::update() {
   for (Layer* layer : layers) {
      layer->update(x_offset, y_offset);
   }
}

void Viewport::render(SDL_Renderer* renderer) {
   for (Layer* layer : layers) {
      layer->render(renderer, x_offset, y_offset);
   }
}

void Viewport::add_layer(Layer* layer) {
   layers.push_back(layer);
}


