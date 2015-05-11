/*
 * geometry: Geometric primitives.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Thursday, Jan 29 2015
 */
#pragma once

#include <cfloat>
#include <cmath>
#include <iostream>
#include <vector>

namespace lost_levels {
   using namespace std;

   template<class T>
   class Rect;

   /**
    * Determine if the two floats provided are equal within the given
    * epsilon.  Default epsilon is FLT_EPSILON.
    */
   inline bool epsilon_equal(float a, float b, float epsilon = FLT_EPSILON) {
      return (fabs(a - b) <= epsilon);
   }

   /**
    * Determine if the two doubles provided are equal within the given
    * epsilon.  Default epsilon is DBL_EPSILON.
    */
   inline bool epsilon_equal(double a, double b, double epsilon = DBL_EPSILON) {
      return (abs(a - b) <= epsilon);
   }

   template<class T>
   inline bool geometric_equal(const T& a, const T& b) {
      return a == b;
   }

   template<>
   inline bool geometric_equal<double>(const double& a, const double& b) {
      return epsilon_equal(a, b);
   }

   template<>
   inline bool geometric_equal<float>(const float& a, const float& b) {
      return epsilon_equal(a, b);
   }

   /**
    * A template class for representing a vector in 2D space.
    *
    * Suggestions:
    *    - Use Vector<int> for device coordinate deltas.
    *    - Use Vector<double> for physics calculations
    *       - Can be converted to Vector<int> with round().
    */
   template <class T>
   class Vector {
   public:
      Vector() :
         vx(0), vy(0) { }

      Vector(T vx, T vy) :
         vx(vx), vy(vy) { }

      /**
       * Calculate the magnitude of the vector as a double.
       */
      double magnitude() const {
         return sqrt((double)vx * (double)vx + (double)vy * (double)vy);
      }

      /**
       * Calculate the z component of the cross product between
       * the given 2D vectors, assuming they are on the same
       * coordinate plane.
       */
      double cross_product(const Vector<T>& B) const {
         Vector<T> nA = normalize();
         Vector<T> nB = B.normalize();

         return (double)(nA.x) * (double)(nB.y) -
                (double)(nA.y) * (double)(nB.x);
      }

      /**
       * Normalize this vector, indicating its direction without
       * indicating its magnitude.  The magnitude of the resulting
       * vector should be close to 1.0.
       */
      Vector<T> normalize() const {
         return (*this) * (1.0 / magnitude());
      }

      Vector<int> round() const {
         return Vector<int>(std::round(vx), std::round(vy));
      }

      bool operator==(const Vector<T>& rhs) const {
         return geometric_equal(vx, rhs.vx) &&
                geometric_equal(vy, rhs.vy);
      }

      bool operator!=(const Vector<T>& rhs) const {
         return ! this->operator==(rhs);
      }

      friend Vector<T> operator+(const Vector<T>& A, const Vector<T>& B) {
         return Vector<T>(A.vx + B.vx, A.vy + B.vy);
      }

      friend Vector<T> operator-(const Vector<T>& A, const Vector<T>& B) {
         return Vector<T>(A.vx - B.vx, A.vy - B.vy);
      }

      friend Vector<T> operator*(const Vector<T>& A, const T& s) {
         return Vector<T>(A.vx * s, A.vy * s);
      }

      friend ostream& operator<<(ostream& out, const Vector<T>& A) {
         out << "Vector(" << A.vx << "x, " << A.vy << "y)";
         return out;
      }

      T vx, vy;
   };

   template <class T>
   class Point {
   public:
      Point(T x, T y) :
         x(x), y(y) { }

      Point() :
         x(0), y(0) { }

      Point<int> round() const {
         return Point<int>(std::round(x), std::round(y));
      }

      bool operator==(const Point<T>& rhs) const {
         return geometric_equal(x, rhs.x) &&
                geometric_equal(y, rhs.y);
      }

      bool operator!=(const Point<T>& rhs) const {
         return ! this->operator==(rhs);
      }

      Point<T>& operator+=(const Vector<T>& A) {
         x += A.vx;
         y += A.vy;
         return *this;
      }

      Point<T>& operator-=(const Vector<T>& A) {
         x -= A.vx;
         y -= A.vy;
         return *this;
      }

      Vector<T> to_vector() const {
         return Vector<T>(x, y);
      }

      friend Vector<T> operator-(const Point<T>& p1, const Point<T>& p2) {
         return Vector<T>(p2.x - p1.x, p2.y - p1.y);
      }

      friend Point<T> operator+(Point<T> p, const Vector<T>& A) {
         return p += A;
      }

      friend Point<T> operator-(Point<T> p, const Vector<T>& A) {
         return p -= A;
      }

      friend ostream& operator<<(ostream& out, const Point<T>& p) {
         out << "Point(" << p.x << "x, " << p.y << "y)";
         return out;
      }

      T x, y;
   };

   template<class T>
   class Line {
   public:
      Line() :
         a(Point<T>()), b(Point<T>()) { }

      Line(const Point<T>& a, const Point<T>& b) :
         a(a), b(b) { }

      Line(const T& ax, const T& ay, const T& bx, const T& by) :
         a(Point<T>(ax, ay)), b(Point<T>(bx, by)) { }

      bool operator==(const Line<T>& rhs) const {
         return a == rhs.a && b == rhs.b;
      }

      bool operator!=(const Line<T>& rhs) const {
         return ! this->operator==(rhs);
      }

      bool intersects(const Line<T>& L, Point<double>* ip = nullptr) const {
         Point<double> s1 = Point<double>(b.x - a.x, b.y - a.y);
         Point<double> s2 = Point<double>(L.b.x - L.a.x, L.b.y - L.a.y);
         double d = (-s2.x * s1.y + s1.x * s2.y);

         if (epsilon_equal(d, 0.0)) {
            return false;
         }

         double s =
            (-s1.y * (a.x - L.a.x) + s1.x * (a.y - L.a.y)) / d;
         double t =
            ( s2.x * (a.y - L.a.y) - s2.y * (a.x - L.a.x)) / d;

         if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
            if (ip != nullptr) {
               ip->x = a.x + (t * s1.x);
               ip->y = a.y + (t * s1.y);
            }

         } else {
            return false;
         }
      }

      double orient_point(const Point<T>& pt) {
         Vector<T> V1 = to_vector();
         Vector<T> V2 = Vector<T>(pt.x - a.x, pt.y - a.y);

         return V1.cross_product(V2);
      }

      bool point_on_line(const Point<T>& pt) const {
         return epsilon_equal(orient_point(pt), 0.0);
      }

      bool segment_intersects_line(const Line<T>& segment) {
         return (point_on_line(segment.a) ||
                 point_on_line(segment.b) ||
                (orient_point(segment.a) < 0 ^
                 orient_point(segment.b) < 0));
      }

      Vector<T> to_vector() const {
         return Vector<T>(b.x - a.x, b.y - a.y);
      }

      friend ostream& operator<<(ostream& out, const Line<T>& L) {
         out << "Line(" << L.a << ", " << L.b << ")";
         return out;
      }

      Point<T> a;
      Point<T> b;
   };

   template <class T>
   class Size {
   public:
      Size() :
         width(0), height(0) { }

      Size(T width, T height) :
         width(width), height(height) { }

      bool operator==(const Size<T>& rhs) const {
         return geometric_equal(width, rhs.width) &&
                geometric_equal(height, rhs.height);
      }

      bool operator!=(const Size<T>& rhs) const {
         return ! this->operator==(rhs);
      }

      Vector<T> x_vector() const {
         return Vector<T>(width, 0);
      }

      Vector<T> y_vector() const {
         return Vector<T>(0, height);
      }

      Vector<T> xy_vector() const {
         return Vector<T>(width, height);
      }

      Size<int> round() const {
         return Size<int>(std::round(width), std::round(height));
      }

      Rect<T> rect() const {
         return Rect<T>(Point<T>(), *this);
      }

      friend ostream& operator<<(ostream& out, const Size<T>& sz) {
         out << "Size(" << sz.width << " x " << sz.height << ")";
         return out;
      }

      T width;
      T height;
   };

   template <class T>
   class Rect {
   public:
      Rect() :
         pt(Point<T>()), sz(Size<T>()) { }
      Rect(const Size<T>& sz) : pt(Point<T>(0, 0)), sz(sz) { }
      Rect(const Point<T>& pt, const Size<T>& sz) :
         pt(pt), sz(sz) { }
      Rect(T width, T height) :
         Rect(Size<T>(width, height)) { }
      Rect(T x, T y, T width, T height) :
         Rect(Point<T>(x, y), Size<T>(width, height)) { }

      Rect<T> move(const Point<T>& p) const {
         return Rect<T>(p, sz);
      }

      Rect<T> translate(const Vector<T>& V) const {
         return move(pt + V);
      }

      bool intersects(const Rect<T>& R2) const {
         return !
            ((pt.x > R2.pt.x + R2.sz.width) ||
             (pt.x + sz.width < R2.pt.x) ||
             (pt.y > R2.pt.y + R2.sz.height) ||
             (pt.y + sz.height < R2.pt.y));
      }

      bool contains(const Point<T>& p) const {
         return p.x >= pt.x &&
                p.x <= pt.x + sz.width &&
                p.y >= pt.y &&
                p.y <= pt.y + sz.height;
      }

      bool contains(const Rect<T>& R2) const {
         return (R2.pt.x >= pt.x &&
                 R2.pt.y >= pt.y &&
                (R2.sz.width + (R2.pt.x - pt.x) <= sz.width) &&
                (R2.sz.height + (R2.pt.y - pt.y) <= sz.height));
      }

      bool intersects(const Line<T>& L) const {
         if (contains(L.a) || contains(L.b)) {
            return true;
         }

         for (Line<T> line : edges()) {
            if (L.intersects(line)) {
               return true;
            }
         }

         return false;
      }

      Rect<int> round() const {
         return Rect<int>(pt.round(), sz.round());
      }

      Line<T> top() const {
         return Line<T>(pt, pt + sz.x_vector());
      }

      Line<T> left() const {
         return Line<T>(pt, pt + sz.y_vector());
      }

      Line<T> bottom() const {
         return Line<T>(pt + sz.y_vector(), pt + sz.xy_vector());
      }

      Line<T> right() const {
         return Line<T>(pt + sz.x_vector(), pt + sz.xy_vector());
      }

      vector<Line<T>> edges() const {
         return {
            top(), left(), bottom(), right()
         };
      }

      Rect<T> tile_rect(const Size<T>& szTile, int tileNum) const {
         const int tilesPerRow = sz.width / szTile.width;

         return Rect<T>(
            pt + Point<T>(
            szTile.width * (tileNum % tilesPerRow),
            szTile.height * (tileNum / tilesPerRow)),
            szTile.width, szTile.height);
      }

      bool operator==(const Rect<T>& rhs) const {
         return pt == rhs.pt && sz == rhs.sz;
      }

      bool operator!=(const Rect<T>& rhs) const {
         return ! this->operator==(rhs);
      }

      static Rect<T> minimum_bound(vector<Point<T>> points) {
         T xmin = 0;
         T ymin = 0;
         T xmax = 0;
         T ymax = 0;

         for (auto pt : points) {
            if (pt.x < xmin) {
               xmin = pt.x;
            } else if (pt.x > xmax) {
               xmax = pt.x;
            }

            if (pt.y < ymin) {
               ymin = pt.y;
            } else if (pt.y > ymax) {
               ymax = pt.y;
            }
         }

         return Rect(xmin, ymin,
            (xmax - xmin), (ymax - ymin));
      }

      static Rect<T> minimum_bound(vector<Rect<T>> rects) {
         vector<Point<T>> points;

         for (auto R : rects) {
            points.push_back(R.pt);
            points.push_back(R.pt + Vector<T>(R.sz.width, R.sz.height));
         }

         return minimum_bound(points);
      }

      friend ostream& operator<<(ostream& out, const Rect<T>& R) {
         out << "Rect(" << R.pt << ", " << R.sz << ")";
         return out;
      }

      Point<T> pt;
      Size<T> sz;
   };
}

