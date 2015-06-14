/*
 * graphics: Primitives and tools for rendering graphics.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, Jan 26 2015
 */
#pragma once
#include "lain/exception.h"
#include "lain/settings.h"
#include "lost_levels/geometry.h"
#include "lost_levels/timer.h"
#include "lost_levels/resource_base.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   class Renderer;

   class GraphicsException : public Exception {
   public:
      using Exception::Exception;
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

   class Image : public ResourceImpl<Resource::Type::IMAGE> {
   public:
      Image(const Rect<int>& rect) : rect(rect) { }

      virtual ~Image() { }
      virtual const Size<int>& get_size() const = 0;

      Point<int> get_tile_point(const Size<int>& szTile, int tileNum) const {
         return get_rect().tile_point(szTile, tileNum);
      }

      Rect<int> get_tile_rect(const Size<int>& szTile, int tileNum) const {
         return get_rect().tile_rect(szTile, tileNum);
      }

   protected:
      const Rect<int>& get_rect() const {
         return rect;
      }

   private:
      Rect<int> rect;
   };

   class Font {
   public:
      Font(Size<int> szChar) : szChar(szChar) { }

      virtual shared_ptr<const Image> get_image() const = 0;

      virtual Size<int> get_size() const {
         return szChar;
      }

      virtual Rect<int> get_char_rect(int c) const {
         return get_image()->get_tile_rect(get_size(), c - get_start_char());
      }

      int get_start_char() const {
         return startChar;
      }

      void set_start_char(int startChar) {
         this->startChar = startChar;
      }

   private:
      int startChar = 0;
      Size<int> szChar;
   };

   class ImageFont : public Font, public ResourceImpl<Resource::Type::FONT> {
   public:
      ImageFont(shared_ptr<const Image> image, const Size<int>& szChar) :
         Font(szChar), image(image) { }

      static shared_ptr<ImageFont> create(shared_ptr<const Image> image,
                                          const Size<int> szChar) {
         return make_shared<ImageFont>(image, szChar);
      }

      shared_ptr<const Image> get_image() const override {
         return image;
      }

      Rect<int> get_char_rect(int c) const override {
         return get_image()->get_tile_rect(get_size(), c - get_start_char());
      }

   private:
      shared_ptr<const Image> image;
   };

   class Animation : public ResourceImpl<Resource::Type::ANIMATION> {
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

         return shared_ptr<Animation>(new Animation(
            image, szFrame, frames, timer, looping));
      }

      shared_ptr<Animation> copy() const {
         return make_shared<Animation>(*this);
      }

      Point<int> get_frame_point() const {
         return image->get_tile_point(szFrame, frames[currentFrame].tileNum);
      }

      Rect<int> get_frame_rect() const {
         return image->get_tile_rect(szFrame, frames[currentFrame].tileNum);
      }

      Size<int> get_size() const {
         return szFrame;
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

   class AnimatedFont : public Font, public ResourceImpl<Resource::Type::ANIMATED_FONT> {
   public:
      AnimatedFont(shared_ptr<const Animation> animation, const Size<int>& szChar) :
         Font(szChar), animation(animation), frameRect(animation->get_size().rect())
      { }

      shared_ptr<const Image> get_image() const override {
         return animation->get_image();
      }

   private:
      shared_ptr<const Animation> animation;
      Rect<int> frameRect;
   };

   class ImageLoader {
   public:
      virtual ~ImageLoader() { }
      virtual shared_ptr<Image> load_image(const string& filename) const = 0;
   };

   class Window {
   public:
      virtual ~Window() { }

      virtual Size<int> get_size() const = 0;
      virtual void set_size(const Size<int>& sz) = 0;

      virtual bool is_fullscreen() const = 0;
      virtual void set_fullscreen(bool fullscreen = true) = 0;

      virtual void set_title(const string& title) = 0;
   };

   class Renderer {
   public:
      virtual ~Renderer() { }

      virtual shared_ptr<Image> load_image(const string& filename) const = 0;

      virtual void clear() = 0;
      virtual void display() = 0;

      virtual void set_draw_color(const Color& color) = 0;
      virtual void set_clip_rect(const Rect<int>& rect) = 0;
      virtual void clear_clip_rect() = 0;

      virtual Size<int> get_logical_size() const = 0;
      virtual void set_logical_size(const Size<int>& sz) = 0;

      virtual void render(shared_ptr<const Image> image,
            const Rect<int>& srcRect,
            const Rect<int>& dstRect) = 0;

      virtual void render(shared_ptr<const Animation> animation,
            const Point<int>& pt) {

         render(animation, Rect<int>(pt, animation->get_frame_rect().sz));
      }

      virtual void render(shared_ptr<const Animation> animation,
            const Rect<int>& dstRect) {
         render(animation->get_image(),
               animation->get_frame_rect(),
               dstRect);
      }

      virtual void render(shared_ptr<const lost_levels::Image> image,
            const Point<int>& pt) {
         render(image, Rect<int>(Point<int>(), image->get_size()),
               Rect<int>(pt, image->get_size()));
      }

      virtual void render(shared_ptr<const lost_levels::Image> image,
            const Rect<int>& dstRect) {
         render(image, Rect<int>(Point<int>(0, 0), image->get_size()),
               dstRect);
      }

      virtual void render_pattern(shared_ptr<const lost_levels::Image> image,
            const Point<int>& scrollPos,
            const Rect<int>& srcRect,
            const Rect<int>& dstRect) {

         Point<int> relativePos = Point<int>(
               -(scrollPos.x % srcRect.sz.width),
               -(scrollPos.y % srcRect.sz.height)) + dstRect.pt.to_vector();

         if (relativePos.x > 0) {
            relativePos.x -= srcRect.sz.width;
         }

         if (relativePos.y > 0) {
            relativePos.y -= srcRect.sz.height;
         }

         Point<int> terminalPoint = dstRect.pt + Vector<int>(
               dstRect.sz.width, dstRect.sz.height);

         set_clip_rect(dstRect);

         for (int y = relativePos.y; y < terminalPoint.y; y += srcRect.sz.height) {
            for (int x = relativePos.x; x < terminalPoint.x; x += srcRect.sz.width) {
               render(image, srcRect, Rect<int>(Point<int>(x, y), srcRect.sz));
            }
         }

         clear_clip_rect();
      }

      virtual void render_pattern(shared_ptr<const lost_levels::Image> image,
            const Point<int>& scrollPos,
            const Rect<int>& srcRect) {
         render_pattern(image, scrollPos, srcRect, Rect<int>(Point<int>(), get_logical_size()));
      }

      virtual void render_pattern(shared_ptr<const lost_levels::Animation> animation,
            const Point<int>& scrollPos,
            const Rect<int>& dstRect) {

         render_pattern(animation->get_image(), scrollPos,
               animation->get_frame_rect(), dstRect);
      }

      virtual void render_pattern(shared_ptr<const lost_levels::Animation> animation,
            const Point<int>& scrollPos) {
         render_pattern(animation, scrollPos, Rect<int>(Point<int>(), get_logical_size()));
      }

      virtual void render_pattern(shared_ptr<const lost_levels::Animation> animation) {
         render_pattern(animation, Point<int>());
      }

      virtual void print_string(const Point<int>& pt,
            shared_ptr<const Font> font, const string& str) {
         Point<int> ptLineStart = pt;
         int col = 0;

         Size<int> sz = font->get_size();
         for (size_t x = 0; x < str.size(); x++) {
            switch(str[x]) {
            case '\n':
               ptLineStart += Vector<int>(0, sz.height);
               col = 0;
               break;

            default:
               render(font->get_image(),
                  font->get_char_rect(str[x]),
                  Rect<int>(ptLineStart + Vector<int>(sz.width * col, 0), sz));
               col++;
               break;
            }
         }
      }
   };
}
