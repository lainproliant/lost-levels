/*
 * graphics_sdl2: SDL2 implementation of graphics interface.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Thursday, February 5th 2015
 */
#pragma once
#include "SDL2/SDL_image.h"
#include "lost_levels/graphics.h"

namespace lost_levels {
   using namespace std;

   namespace sdl2 {
      class Image : public lost_levels::Image {
      public:
         Image(SDL_Texture* texture) : texture(texture) {
            SDL_QueryTexture(texture, nullptr, nullptr,
               &sz.width, &sz.height);
         }

         virtual ~Image() { }

         const Size<int>& get_size() const override {
            return sz;
         }

         SDL_Texture* get_sdl_texture() const {
            return texture;
         }

      private:
         SDL_Texture* texture;
         Size<int> sz;
      };

      class Window : public lost_levels::Window {
      public:
         Window(SDL_Window* window, const Size<int>& sz) :
            window(window), sz(sz) { }

         virtual ~Window() {
            SDL_DestroyWindow(window);
         }

         static const Size<int>& default_size() {
            static Size<int> defaultSize = Size<int>(1200, 800);
            return defaultSize;
         }

         Size<int> get_size() const override {
            Size<int> sz;
            SDL_GetWindowSize(window, &sz.width, &sz.height);
            return sz;
         }

         void set_size(const Size<int>& sz) override {
            SDL_SetWindowSize(window, sz.width, sz.height);
         }

         bool is_fullscreen() const override {
            return fullscreen;
         }

         void set_fullscreen(bool fullscreen = true) override {
            if (fullscreen) {
               SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

            } else {
               SDL_SetWindowFullscreen(window, 0);
            }

            this->fullscreen = fullscreen;
         }

         SDL_Window* get_sdl_window() const {
            return window;
         }

      private:
         SDL_Window* window;
         Size<int> sz;
         bool fullscreen = false;
      };

      class Renderer : public lost_levels::Renderer {
      public:
         Renderer(SDL_Renderer* renderer) :
            renderer(renderer) { }

         virtual ~Renderer() {
            SDL_DestroyRenderer(renderer);
         }

         void clear() override {
            SDL_RenderClear(renderer);
         }

         void display() override {
            SDL_RenderPresent(renderer);
         }

         void render(shared_ptr<const Animation> animation,
                     const Point<int>& pt) override {

            render(animation, Rect<int>(pt, animation->get_frame_rect().sz));
         }

         void render(shared_ptr<const Animation> animation,
                     const Rect<int>& dstRect) override {
            render(animation->get_image(),
                   animation->get_frame_rect(),
                   dstRect);
         }

         void render(shared_ptr<const lost_levels::Image> imageIn,
                     const Point<int>& pt) override {
            shared_ptr<const Image> image =
               static_pointer_cast<const Image>(imageIn);

            render(image, Rect<int>(Point<int>(), image->get_size()),
                   Rect<int>(pt, image->get_size()));
         }

         void render(shared_ptr<const lost_levels::Image> image,
                     const Rect<int>& dstRect) override {
            render(image, Rect<int>(Point<int>(0, 0), image->get_size()),
               dstRect);
         }

         void render(shared_ptr<const lost_levels::Image> imageIn,
               const Rect<int>& src,
               const Rect<int>& dst) override {
            shared_ptr<const Image> image =
               static_pointer_cast<const Image>(imageIn);

            SDL_RenderCopy(renderer, image->get_sdl_texture(),
                  (SDL_Rect*)&src, (SDL_Rect*)&dst);
         }

         shared_ptr<lost_levels::Image> load_image(const string& filename) const override {
            SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());

            if (texture == nullptr) {
               throw GraphicsException(tfm::format("Failed to load image from file '%s': %s",
                  filename, string(SDL_GetError())));
            }

            return shared_ptr<lost_levels::Image>(new Image(texture));
         }

         void set_draw_color(const Color& color) override {
            SDL_SetRenderDrawColor(renderer,
                  color.r, color.g, color.b, color.a);
         }

         Size<int> get_logical_size() const override {
            Size<int> sz;
            SDL_RenderGetLogicalSize(renderer, &sz.width, &sz.height);
            return sz;
         }

         void set_logical_size(const Size<int>& sz) override {
            SDL_RenderSetLogicalSize(renderer, sz.width, sz.height);
         }

      private:
         SDL_Renderer* renderer;
      };

      inline shared_ptr<lost_levels::Window> create_window(
            const Size<int>& sz = Window::default_size(),
            Uint32 flags = SDL_WINDOW_SHOWN) {
         SDL_Window* window = SDL_CreateWindow("",
               SDL_WINDOWPOS_CENTERED,
               SDL_WINDOWPOS_CENTERED,
               sz.width, sz.height, flags);

         if (window == nullptr) {
            throw GraphicsException(tfm::format(
               "Failed to create sdl2::Window: %s", string(SDL_GetError())));
         }

         return shared_ptr<lost_levels::Window>(
               new Window(window, Window::default_size()));
      }

      inline shared_ptr<lost_levels::Renderer> create_renderer(
            shared_ptr<lost_levels::Window> windowIn,
            Uint32 flags = SDL_RENDERER_ACCELERATED |
                           SDL_RENDERER_PRESENTVSYNC) {

         SDL_Renderer* renderer;
         shared_ptr<Window> window = dynamic_pointer_cast<Window>(windowIn);
         if (window == nullptr) {
            throw GraphicsException("sdl2::Renderer requires an sdl2::Window.");
         }

         renderer = SDL_CreateRenderer(window->get_sdl_window(), -1, flags);
         if (renderer == nullptr) {
            throw GraphicsException(tfm::format(
               "Failed to create sdl2::Renderer: %s",
               string(SDL_GetError())));
         }

         return shared_ptr<lost_levels::Renderer>(
            new Renderer(renderer));
      }
   }
}

