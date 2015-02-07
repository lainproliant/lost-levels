/*
 * graphics: Primitives and tools for rendering graphics.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, Jan 26 2015
 */
#pragma once
#include "lain/exception.h"
#include "lost_levels/geometry.h"
#include "lost_levels/timer.h"
#include "lost_levels/settings.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   class GraphicsException : public Exception {
   public:
      GraphicsException(const string& message) :
         Exception(message) { }
   };

   class Color {
   public:
      Color() :
         r(0), g(0), b(0), a(0) { }
      Color(uint8_t r, uint8_t g, uint8_t b) :
         r(r), g(g), b(b), a(0) { }
      Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
         r(r), g(g), b(b), a(a) { }
uint8_t r, g, b, a;
   };

   class Image {
   public:
      virtual ~Image() { }
      virtual const Size<int>& get_size() const = 0;

      Rect<int> get_tile_rect(const Size<int>& szTile, int tileNum) const {
         const int tilesPerRow = get_size().width / szTile.width;

         return Rect<int>(
            szTile.width * (tileNum % tilesPerRow),
            szTile.height * (tileNum / tilesPerRow),
            szTile.width, szTile.height);
      }
   };

   class Animation {
   public:
      class Frame {
      public:
         Frame(int tileNum, uint32_t duration) :
            tileNum(tileNum), duration(duration) { }

         int tileNum;
         uint32_t duration;
      };

      static vector<Frame> parse_frames(const vector<string>& frameExprs) {
         vector<Frame> frames;

         for (auto frameExpr : frameExprs) {
            vector<string> tuple;
            str::split(tuple, frameExpr, ",");

            if (tuple.size() != 2) {
               throw GraphicsException(tfm::format(
                  "Invalid frame expression: %s", frameExpr));
            }
            
            frames.push_back(Frame(stoi(tuple[0]), stoi(tuple[1])));
         }

         return frames;
      }

      Animation(const Animation& rhs) :
         timer(rhs.timer->copy()), image(rhs.image),
         szFrame(rhs.szFrame), frames(rhs.frames),
         looping(rhs.looping), numFrames(rhs.frames.size()) { }

      static shared_ptr<Animation> create(
         shared_ptr<const Image> image,
         const Size<int>& szFrame, const vector<Frame>& frames,
         shared_ptr<const Timer<unsigned int>>& timer,
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
         return image->get_tile_rect(szFrame, frames[currentFrame].tileNum);
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
         timer->set_interval(frames[currentFrame].duration);
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
         const Size<int>& szFrame, const vector<Frame>& frames,
         shared_ptr<const Timer<unsigned int>> timer,
         bool looping = true) :

         timer(timer->relative_timer()),
         image(image), szFrame(szFrame),
         frames(frames), looping(looping),
         numFrames(frames.size()) { }

      shared_ptr<Timer<unsigned int>> timer;
      shared_ptr<const Image> image;
      Size<int> szFrame;
      vector<Frame> frames;
      bool looping;
      size_t numFrames;

      unsigned int currentFrame = 0;
   };

   class Window {
   public:
      virtual ~Window() { }

      virtual Size<int> get_size() const = 0;
      virtual void set_size(const Size<int>& sz) = 0;

      virtual bool is_fullscreen() const = 0;
      virtual void set_fullscreen(bool fullscreen = true) = 0;
   };

   class Renderer {
   public:
      virtual ~Renderer() { }

      virtual shared_ptr<Image> load_image(const string& filename) const = 0;

      virtual void clear() = 0;
      virtual void display() = 0;
      virtual void render(shared_ptr<const Animation> animation,
         const Point<int>& pt) = 0;
      virtual void render(shared_ptr<const Animation> animation,
         const Rect<int>& dstRect) = 0;
      virtual void render(shared_ptr<const Image> image,
         const Point<int>& pt) = 0;
      virtual void render(shared_ptr<const Image> image,
         const Rect<int>& dstRect) = 0;
      virtual void render(shared_ptr<const Image> image,
         const Rect<int>& srcRect,
         const Rect<int>& dstRect) = 0;

      virtual void set_draw_color(const Color& color) = 0;

      virtual Size<int> get_logical_size() const = 0;
      virtual void set_logical_size(const Size<int>& sz) = 0;
   };
}
