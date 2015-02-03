/*
 * graphics: Primitives and tools for rendering graphics.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, Jan 26 2015
 */
#pragma once
#include "common.h"
#include "SDL2/SDL_image.h"
#include "lost_levels/geometry.h"
#include "lost_levels/timer.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   class Renderer;

   class GraphicsException : public Exception {
   public:
      GraphicsException(const string& message) :
         Exception(message) { }
   };

   class Color {
   public:
      Color() :
         r(0), g(0), b(0), a(0) { }
      Color(Uint8 r, Uint8 g, Uint8 b) :
         r(r), g(g), b(b), a(0) { }
      Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a) :
         r(r), g(g), b(b), a(a) { }

      Uint8 r, g, b, a;
   };

   class Image {
   public:
      friend class Renderer;

      Image(SDL_Texture* texture) :
         texture(texture) {
         SDL_QueryTexture(texture, nullptr, nullptr,
            &sz.width, &sz.height);
      }

      virtual ~Image() { }

      const Size<int>& get_size() const {
         return sz;
      }

   private:
      SDL_Texture* texture;
      Size<int> sz;
   };

   class Animation {
   public:
      friend class Renderer;

      Animation(const Animation& rhs) :
         timer(rhs.timer->copy()), image(rhs.image),
         szFrame(rhs.szFrame), frames(rhs.frames),
         looping(rhs.looping), numFrames(rhs.frames.size()) { }

      static shared_ptr<Animation> create(
         shared_ptr<const Image> image,
         const Size<int>& szFrame, const vector<int>& frames,
         shared_ptr<const Timer<Uint32>>& timer,
         bool looping = false) {

         if (frames.size() < 1) {
            throw GraphicsException("Animation must have at least 1 frame.");
         }

         const Size<int>& sz = image->get_size();
         if (sz.width % szFrame.width != 0) {
            throw GraphicsException(tfm::format(
               "Image width %d is not a multiple of frame width %d.",
               sz.width, szFrame.width));
         }

         if (sz.height % szFrame.height != 0) {
            throw GraphicsException(tfm::format(
               "Image height %d is not a multiple of frame height %d.",
               sz.height, szFrame.height));
         }

         return shared_ptr<Animation>(new Animation(
            image, szFrame, frames, timer, looping));
      }

      shared_ptr<Animation> copy() const {
         return make_shared<Animation>(*this);
      }

      Rect<int> get_frame_rect() const {
         const Size<int>& szImage = image->get_size();

         return Rect<int>(
            Point<int>(
               (szFrame.width * currentFrame) % szImage.width,
               (szFrame.height * ((szFrame.width * currentFrame) / szImage.width))),
            szFrame);
      }

      shared_ptr<const Image> get_image() const {
         return image;
      }

      bool is_complete() const {
         if (! looping)
            return false;

         return currentFrame == frames.size() - 1;
      }

      void reset() {
         currentFrame = 0;
         timer->reset();
      }

      void pause() {
         timer->pause();
      }

      void start() {
         timer->set_interval(frames[currentFrame]);
         timer->start();
      }

      void update() {
         if (timer->update()) {
            if (looping) {
               currentFrame = (currentFrame + 1) % numFrames;

            } else {
               currentFrame = min<size_t>(
                     currentFrame + 1, numFrames - 1);
            }
         }
      }

   private:
      Animation(shared_ptr<const Image> image,
         const Size<int>& szFrame, const vector<int>& frames,
         shared_ptr<const Timer<Uint32>> timer,
         bool looping = true) :

         timer(timer->relative_timer()),
         image(image), szFrame(szFrame),
         frames(frames), looping(looping),
         numFrames(frames.size()) { }

      shared_ptr<Timer<Uint32>> timer;
      shared_ptr<const Image> image;
      Size<int> szFrame;
      vector<int> frames;
      bool looping;
      size_t numFrames;

      unsigned int currentFrame = 0;
   };

   class Window {
   public:
      friend class Renderer;

      class Builder {
      public:
         Builder() { }
         virtual ~Builder() { }

         Builder& fullscreen() {
            fullscr = true;
            return *this;
         }

         Builder& with_size(const Size<int>& size) {
            sz = size;
            return *this;
         }

         Builder& with_title(const string& title) {
            this->title = title;
            return *this;
         }

         shared_ptr<Window> create() const {
            Uint32 flags = SDL_WINDOW_SHOWN;

            if (fullscr) {
               flags |= SDL_WINDOW_FULLSCREEN;
            }

            SDL_Window* window = SDL_CreateWindow(title.c_str(),
               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
               sz.width, sz.height, flags);

            if (window == nullptr) {
               throw GraphicsException(tfm::format(
                  "Failed to create window: %s", string(SDL_GetError())));
            }

            return shared_ptr<Window>(new Window(window, sz));
         }

      private:
         string title = "Lost Levels Engine";
         Size<int> sz = Size<int>(1280, 800);
         bool fullscr = false;
      };

      virtual ~Window() {
         SDL_DestroyWindow(window);
      }

      const Size<int>& get_size() const {
         return sz;
      }

   private:
      Window(SDL_Window* window, const Size<int>& sz) :
         window(window), sz(sz) { }

      SDL_Window* window;
      Size<int> sz;
   };

   class Renderer {
   public:
      virtual ~Renderer() {
         SDL_DestroyRenderer(renderer);
      }

      class Builder {
      public:
         Builder(shared_ptr<Window> window) :
            window(window) { }
         virtual ~Builder() { }

         Builder& with_logical_size(const Size<int>& logicalSize) {
            this->logicalSize = logicalSize;
            return *this;
         }

         Builder& with_acceleration() {
            flags |= SDL_RENDERER_ACCELERATED;
            return *this;
         }

         Builder& with_vsync() {
            flags |= SDL_RENDERER_PRESENTVSYNC;
            return *this;
         }

         shared_ptr<Renderer> create() const {
            SDL_Renderer* renderer = SDL_CreateRenderer(
                  window->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            if (renderer == nullptr) {
               throw GraphicsException(tfm::format(
                  "Failed to create renderer: %s",
                  string(SDL_GetError())));
            }

            cout << "Logical Size: " << logicalSize << endl;

            if (logicalSize.width > 0 || logicalSize.height > 0) {
               SDL_RenderSetLogicalSize(renderer,
                  logicalSize.width, logicalSize.height);
            }

            return shared_ptr<Renderer>(new Renderer(window, renderer));
         }

      private:
         shared_ptr<Window> window;
         Size<int> logicalSize = Size<int>(0, 0);
         Uint32 flags;
      };

      void clear() {
         SDL_RenderClear(renderer);
      }

      void display() {
         SDL_RenderPresent(renderer);
      }

      void render(shared_ptr<const Animation> animation,
                  const Point<int>& pt) {

         render(animation, Rect<int>(pt, animation->szFrame));
      }

      void render(shared_ptr<const Animation> animation,
                  const Rect<int>& dstRect) {
         render(animation->image,
                animation->get_frame_rect(),
                dstRect);
      }

      void render(shared_ptr<const Image> image,
                  const Point<int>& pt) {

         render(image, Rect<int>(Point<int>(), image->sz),
                Rect<int>(pt, image->sz));
      }

      void render(shared_ptr<const Image> image,
            const Rect<int>& src,
            const Rect<int>& dst) {

         SDL_RenderCopy(renderer, image->texture,
               (SDL_Rect*)&src, (SDL_Rect*)&dst);
      }

      shared_ptr<Image> load_image(const string& filename) {
         SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());

         if (texture == nullptr) {
            throw GraphicsException(tfm::format("Failed to load image from file '%s': %s",
               filename, string(SDL_GetError())));
         }

         return make_shared<Image>(texture);
      }

      void set_draw_color(const Color& color) {
         SDL_SetRenderDrawColor(renderer,
               color.r, color.g, color.b, color.a);
      }

   private:
      Renderer(shared_ptr<Window> window, SDL_Renderer* renderer) :
         window(window), renderer(renderer) { }

      shared_ptr<Window> window;
      SDL_Renderer* renderer;
   };
}
