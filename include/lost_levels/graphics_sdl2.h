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
      class ImageLoader;

      inline Size<int> get_texture_size(SDL_Texture* texture) {
         Size<int> sz;
         SDL_QueryTexture(texture, nullptr, nullptr,
            &sz.width, &sz.height);
         return sz;
      }

      class Image : public lost_levels::Image {
      public:
         Image(SDL_Texture* texture) :
            lost_levels::Image(get_texture_size(texture).rect()), texture(texture)
         { }

         virtual ~Image() {
            SDL_DestroyTexture(texture);
         }

         const Size<int>& get_size() const override {
            return get_rect().sz;
         }

         SDL_Texture* get_sdl_texture() const {
            return texture;
         }

      private:
         SDL_Texture* texture;
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

         void set_title(const string& title) override {
            SDL_SetWindowTitle(window, title.c_str());
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
         friend class ImageLoader;

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
         
         void draw_rect(const Rect<int>& rect) override {
            SDL_RenderDrawRect(renderer, (SDL_Rect*)&rect);
         }

         void fill_rect(const Rect<int>& rect) override {
            SDL_RenderFillRect(renderer, (SDL_Rect*)&rect);
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

         void set_clip_rect(const Rect<int>& rect) override {
            SDL_RenderSetClipRect(renderer, (SDL_Rect*)&rect);
         }

         void clear_clip_rect() override {
            SDL_RenderSetClipRect(renderer, nullptr);
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

      class ImageLoader : public lost_levels::ImageLoader {
      public:
         ImageLoader(shared_ptr<Renderer> renderer) : renderer(renderer) { }

         shared_ptr<lost_levels::Image> load_image(const string& filename) const override {
            SDL_Texture* texture = IMG_LoadTexture(renderer->renderer, filename.c_str());

            if (texture == nullptr) {
               throw GraphicsException(tfm::format("Failed to load image from file '%s': %s",
                  filename, string(SDL_GetError())));
            }

            return shared_ptr<lost_levels::Image>(new Image(texture));
         }

      private:
         shared_ptr<Renderer> renderer;
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
         SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
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

      inline shared_ptr<lost_levels::ImageLoader> create_image_loader(
            shared_ptr<lost_levels::Renderer> rendererIn) {
         shared_ptr<Renderer> renderer = dynamic_pointer_cast<Renderer>(rendererIn);
         if (renderer == nullptr) {
            throw GraphicsException("sdl2::ImageLoader requires an sdl2::ImageLoader");
         }

         return shared_ptr<lost_levels::ImageLoader>(new ImageLoader(renderer));
      }
   }
}

