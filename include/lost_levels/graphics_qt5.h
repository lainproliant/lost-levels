/*
 * graphics_qt5: Qt5 Widget implementation of graphics interface.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Saturday, Feburary 7th 2015
 */
#pragma once
#include "lost_levels/graphics.h"
#include <QPainter>

namespace lost_levels {
   using namespace std;

   namespace qt5 {
      class Image : public lost_levels::Image {
      public:
         Image(QImage& image) : image(image) {
            sz = Size<int>(image.size.width, image.size.height);
         }

         const Size<int>& get_size() const override {
            return sz;
         }

         const QImage& get_qt5_image() const {
            return image;
         }

         virtual ~Image() { }

      private:
          QImage image;
          Size<int> sz;
      };

      class Renderer : public lost_levels::Renderer {
      public:
          Renderer(QPainter& painter) : painter(painter) {
              drawColor = QColor();
              qrect = QRect(0, 0,
                           painter.device()->width,
                           painter.device()->height);
              rect = Rect<int>(0, 0,
                               painter.device()->width,
                               painter.device()->height);
          }

          virtual ~Renderer() { }

          void clear() override {
              painter.fillRect(qrect, drawColor);
          }

          void render(shared_ptr<const lost_levels::Image> imageIn,
                      const Rect<int>& srcRectIn,
                      const Rect<int>& dstRectIn) override {
              shared_ptr<const Image> image =
                  static_pointer_cast<const Image>(imageIn);

              QRect srcRect = QRect(
                  srcRectIn.pt.x,
                  srcRectIn.pt.y,
                  srcRectIn.sz.width,
                  srcRectIn.sz.height);
              QRect dstRect = QRect(
                  dstRectIn.pt.x,
                  dstRectIn.pt.y,
                  dstRectIn.sz.width,
                  dstRectIn.sz.height);

              painter.drawImage(dstRect, image->get_qt5_image(),
                                srcRect);
          }

          shared_ptr<lost_levels::Image> load_image(const string& filename) const override {
              QImage image = QImage(filename);
              if (image.isNull()) {
                  if (texture == nullptr) {
                     throw GraphicsException(tfm::format("Failed to load image from file '%s'.", filename));
                  }
              }

              return make_shared<lost_levels::Image>(new Image(image));
          }

          void set_draw_color(const Color& color) override {
              drawColor = QColor(color.r, color.g,
                                 color.b, color.a);
          }

          Size<int> get_logical_size() const override {
              return rect.sz;
          }

          void set_logical_size(const Size<int>&) override {
              /* operation not supported. */
              cerr << "WARNING: qt5::Renderer::set_logical_size not supported." << endl;
          }

      private:
          QPainter painter;
          QColor drawColor;
          QRect qrect;
          Rect<int> rect;
      };
   }
}
