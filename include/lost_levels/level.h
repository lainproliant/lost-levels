/*
 * level: Data structures to represent level data.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Sunday, May 17 2015
 */
#pragma once

#include "geometry.h"
#include "graphics.h"

namespace lost_levels {
   using namespace std;

   class Layer {
   public:
      Layer(const Size<int>& sz) : sz(sz) { }

      virtual void initialize() = 0;
      virtual void update() = 0;
      virtual void paint(shared_ptr<Renderer> renderer) const = 0;

      virtual const Size<int>& get_size() const {
         return sz;
      }

   private:
      Size<int> sz;
   };

   class ImageLayer : public Layer {
   public:
      ImageLayer(shared_ptr<const Image> image) :
         Layer(image->get_size()), image(image),
         position(Point<float>()),
         scrollVelocity(Vector<float>()) { }

      virtual void initialize() override { }

      virtual void update() override {
         position += scrollVelocity;
      }

      virtual void paint(shared_ptr<Renderer> renderer) const override {
         renderer->render_pattern(image, position.round(),
               Rect<int>(Point<int>(), image->get_size()));
      }

      void set_position(const Point<float>& pt) {
         position = pt;
      }

      void set_scroll_velocity(const Vector<float>& v) {
         scrollVelocity = v;
      }

   protected:
      shared_ptr<const Image> image;
      Point<float> position;
      Vector<float> scrollVelocity;
   };

   class AnimatedImageLayer : public ImageLayer {
   public:
      AnimatedImageLayer(shared_ptr<Animation> animation) :
         ImageLayer(animation->get_image()),
         animation(animation) { }

      void update() override {
         ImageLayer::update();
         animation->update();
      }

      void paint(shared_ptr<Renderer> renderer) const override {
         renderer->render_pattern(animation, position.round());
      }

   private:
      shared_ptr<Animation> animation;
   };

   class Block {
   public:
      Block(int id, const Point<int> position) :
         id(id), position(position) { }

      int get_id() const {
         return id;
      }

      const Point<int>& get_position() const {
         return position;
      }

   private:
      int id;
      Point<int> position;
   };
}

