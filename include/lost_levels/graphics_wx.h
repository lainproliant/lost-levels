/*
 * graphics_wx: wxWidgets implementation of graphics interface.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Thursday, February 5th 2015
 *
 * NOTE: Due to the ephemeral nature of wxDC, a new Renderer will need to be
 * created for each wxDC.  For instance, you will need to create a Renderer
 * each time your OnDraw() method is invoked.
 *
 * EXAMPLE:
 *
 * void MyWidget::OnDraw(wxDC& dc) {
 *    Renderer renderer = wx::create_renderer(dc);
 *    ...
 * }
 *
 */
#pragma once

#include <wx/wx.h>
#include <wx/image.h>
#include "lost_levels/graphics.h"

namespace lost_levels {
   using namespace std;

   namespace wx {
      inline Size<int> get_image_size(const wxImage& image) {
         return Size<int>(
            image.GetWidth(),
            image.GetHeight());
      }

      inline wxColour wx_colour(const Color& color) {
         return wxColour(color.r, color.g, color.b, color.a);
      }

      inline wxPoint wx_point(const Point<int>& pt) {
         return wxPoint(pt.x, pt.y);
      }

      inline wxSize wx_size(const Size<int>& sz) {
         return wxSize(sz.width, sz.height);
      }

      inline wxRect wx_rect(const Rect<int>& rect) {
         return wxRect(wx_point(rect.pt), wx_size(rect.sz));
      }

      class Image : public lost_levels::Image {
      public:
         Image(const wxImage& image) :
            lost_levels::Image(get_image_size(image).rect())
         {
            bitmap = new wxBitmap(image);
            dc = new wxMemoryDC(*bitmap);
         }

         virtual ~Image() {
            delete dc;
            delete bitmap;
         }

         const Size<int>& get_size() const override {
            return get_rect().sz;
         }

         wxDC* get_dc() const {
            return dc;
         }

      private:
         wxMemoryDC* dc;
         wxBitmap* bitmap;
      };

      class Renderer : public lost_levels::Renderer {
      public:
         Renderer(wxDC& dc) : dc(dc) {
            const wxSize dcsz = dc.GetSize();
            logicalSize = Size<int>(dcsz.GetWidth(), dcsz.GetHeight());
         }

         virtual ~Renderer() { }

         void clear() override {
            const Size<int>& sz = get_logical_size();
            dc.DrawRectangle(0, 0, sz.width, sz.height);
         }

         /**
          * NOTE: This method is not necessary in wxWidgets,
          * call wxWindow::SetDoubleBuffered().
          */
         void display() override { }

         void render(shared_ptr<const lost_levels::Image> imageIn,
               const Rect<int>& src,
               const Rect<int>& dst) override {
            shared_ptr<const Image> image =
               static_pointer_cast<const Image>(imageIn);

            dc.StretchBlit(dst.pt.x, dst.pt.y, dst.sz.width, dst.sz.height, image->get_dc(),
                           src.pt.x, src.pt.y, src.sz.width, src.sz.height,
                           wxCOPY, true);
         }

         void set_draw_color(const Color& color) override {
            dc.SetBrush(wxBrush(wx_colour(color)));
         }

         void set_clip_rect(const Rect<int>& rect) override {
            dc.SetClippingRegion(wx_rect(rect));
         }

         void clear_clip_rect() override {
            dc.DestroyClippingRegion();
         }

         Size<int> get_logical_size() const override {
            return logicalSize;
         }

         void set_logical_size(const Size<int>& sz) override {
            const wxSize dcsz = dc.GetSize();

            if (dcsz == wx_size(sz)) {
               dc.SetLogicalScale(1, 1);

            } else {
               dc.SetLogicalScale(
                  ((double) sz.width) / ((double) dcsz.GetWidth()),
                  ((double) sz.height) / ((double) dcsz.GetHeight()));
            }

            logicalSize = sz;
         }

      private:
         wxDC& dc;
         Size<int> logicalSize;
      };

      class ImageLoader : public lost_levels::ImageLoader {
      public:
         ImageLoader() { }

         shared_ptr<lost_levels::Image> load_image(const string& filename) const override {
            wxImage image;
            if (! image.LoadFile(filename)) {
               throw GraphicsException(tfm::format("Failed to load image from file: '%s'", filename));
            }

            return shared_ptr<lost_levels::Image>(new Image(image));
         }
      };

      inline shared_ptr<lost_levels::Renderer> create_renderer(wxDC& dc) {
         return shared_ptr<lost_levels::Renderer>(new Renderer(dc));
      }

      inline shared_ptr<lost_levels::ImageLoader> create_image_loader() {
         return shared_ptr<lost_levels::ImageLoader>(new ImageLoader());
      }
   }
}

