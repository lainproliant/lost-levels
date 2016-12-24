/*
 * geometry: Geometric primitives.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Thursday, Jan 29 2015
 */
#pragma once

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include "lain/algorithms.h"
#include "lain/settings.h"
#include "lain/util.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   template<class T>
   class Rect;
   
   template<class T>
   class Polygon;

   /**
    * Determine if the two floats provided are equal within the given
    * epsilon.  Default epsilon is FLT_EPSILON.
    */
   inline bool epsilon_equal(float a, float b, float epsilon = FLT_EPSILON) {
      return (fabs(a - b) < epsilon);
   }

   /**
    * Determine if the two doubles provided are equal within the given
    * epsilon.  Default epsilon is DBL_EPSILON.
    */
   inline bool epsilon_equal(double a, double b, double epsilon = DBL_EPSILON) {
      return (abs(a - b) < epsilon);
   }
   
   /**
    * Overload of geometric_equal for non-double types.
    */
   template<class T>
   inline bool geometric_equal(const T& a, const T& b) {
      return a == b;
   }
   
   /**
    * Overload of geometric_equal for double type using epsilon_equal.
    */
   template<>
   inline bool geometric_equal<double>(const double& a, const double& b) {
      return epsilon_equal(a, b);
   }

   /**
    * Overload of geometric_equal for float type using epsilon_equal.
    */
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
   class Vector { public:
      /**
       * Creates an initial vector with 0 magnitude.
       */
      Vector() :
         vx(0), vy(0) { }
   
      /**
       * Creates an initial vector with the given x and y components.
       */
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
       * Calculate the dot product of the given vectors.
       */
      double dot_product(const Vector<T>& B) const {
         return inner_product({vx, vy}, 2, {B.vx, B.vy}, 0);
      }

      /**
       * Normalize this vector, indicating its direction without
       * indicating its magnitude.  The magnitude of the resulting
       * vector should be close to 1.0.
       */
      Vector<T> normalize() const {
         return (*this) * (1.0 / magnitude());
      }

      /**
       * Round this vector to the nearest integer values for
       * each component.  Useful for converting floating point
       * kinematics to screen coordinates.
       */
      Vector<int> round() const {
         return Vector<int>(std::round(vx), std::round(vy));
      }
   
      /**
       * Compare two vectors to determine if they are geometrically equal.
       */
      bool operator==(const Vector<T>& rhs) const {
         return geometric_equal(vx, rhs.vx) &&
                geometric_equal(vy, rhs.vy);
      }
      
      /**
       * Compare two vectors to determine if they are not geometrically equal.
       */
      bool operator!=(const Vector<T>& rhs) const {
         return ! this->operator==(rhs);
      }
      
      /**
       * Add the components of one vector to another, e.g. adding acceleration
       * to velocity vectors or adding velocity to a displacement vector.
       */
      Vector<T> operator+=(const Vector<T>& rhs) {
         (*this) = (*this) + rhs;
         return (*this);
      }
   
      /**
       * Subtract the components of one vector from another.
       */
      Vector<T> operator-=(const Vector<T>& rhs) {
         (*this) = (*this) - rhs;
         return (*this);
      }
   
      /**
       * Convert the vector to a Settings object for printing.
       */
      Settings json() const {
         Settings json;
         json.set<T>("vx", vx);
         json.set<T>("vy", vy);
         return json;
      }
   
      /**
       * Add two vectors together.
       */
      friend Vector<T> operator+(const Vector<T>& A, const Vector<T>& B) {
         return Vector<T>(A.vx + B.vx, A.vy + B.vy);
      }
      
      /**
       * Subtract two vectors from each other.
       */
      friend Vector<T> operator-(const Vector<T>& A, const Vector<T>& B) {
         return Vector<T>(A.vx - B.vx, A.vy - B.vy);
      }
   
      /**
       * Multiply a vector by a given scalar constant.
       */
      friend Vector<T> operator*(const Vector<T>& A, const T& s) {
         return Vector<T>(A.vx * s, A.vy * s);
      }
      
      /**
       * Write the vector to the given output stream.
       */
      friend ostream& operator<<(ostream& out, const Vector<T>& A) {
         out << "Vector" << A.json();
         return out;
      }
   
      /**
       * The x and y components of the vector.
       */
      T vx, vy;
   };

   /**
    * Represents a point.  This can be a screen coordinate, physical
    * coordinate, or a location in a 2d grid.
    *
    * Note that x and y elements are public.
    */
   template <class T>
   class Point {
   public:
      /**
       * Create a point at the origin of a cartesian plane.
       */
      Point() :
         x(0), y(0) { }
      
      /**
       * Create a point at the given x and y location.
       */
      Point(T x, T y) :
         x(x), y(y) { }

      /**
       * Round this point to the nearest integer values for
       * each coordinate.  Useful for converting floating point
       * locations to screen coordinates.
       */
      Point<int> round() const {
         return Point<int>(std::round(x), std::round(y));
      }
      
      /**
       * Compare two points to determine if they are geometrically equal.
       */
      bool operator==(const Point<T>& rhs) const {
         return geometric_equal(x, rhs.x) &&
                geometric_equal(y, rhs.y);
      }
      
      /**
       * Compare two points to determine if they are not geometrically equal.
       */
      bool operator!=(const Point<T>& rhs) const {
         return ! this->operator==(rhs);
      }
      
      /**
       * Add the given vector components to this point.
       */
      Point<T>& operator+=(const Vector<T>& A) {
         x += A.vx;
         y += A.vy;
         return *this;
      }
      
      /**
       * Subtract the given vector components from this point.
       */
      Point<T>& operator-=(const Vector<T>& A) {
         x -= A.vx;
         y -= A.vy;
         return *this;
      }

      /**
       * Convert this point into a Vector with components equal to
       * the distance from the origin (0,0) to this point.
       */
      Vector<T> to_vector() const {
         return Vector<T>(x, y);
      }
      
      /**
       * Convert the point to a Settings object for printing.
       */
      Settings json() const {
         Settings json;
         json.set<T>("x", x);
         json.set<T>("y", y);
         return json;
      }

      /**
       * Find the vector between two points.
       */
      friend Vector<T> operator-(const Point<T>& p1, const Point<T>& p2) {
         return Vector<T>(p2.x - p1.x, p2.y - p1.y);
      }
      
      /**
       * Add the given vector components to a point.
       */
      friend Point<T> operator+(Point<T> p, const Vector<T>& A) {
         return p += A;
      }
      
      /**
       * Subtract the given vector components from a point.
       */
      friend Point<T> operator-(Point<T> p, const Vector<T>& A) {
         return p -= A;
      }
      
      /**
       * Write the point to the given output stream.
       */
      friend ostream& operator<<(ostream& out, const Point<T>& p) {
         out << "Point" << p.json();
         return out;
      }
      
      /**
       * The x and y displacement values from the cartesian origin
       * for this point.
       */
      T x, y;
   };

   /**
    * Represents a line or line segment connecting given two points.
    *
    * Note that a and b point elements are public.
    */
   template<class T>
   class Line {
   public:
      /**
       * Creates a 0-length line with both points at the origin.
       */
      Line() :
         a(Point<T>()), b(Point<T>()) { }
   
      /**
       * Contains a line between the two given points.
       */
      Line(const Point<T>& a, const Point<T>& b) :
         a(a), b(b) { }
      
      /**
       * Contains a line between the two given points identified by
       * the given x and y displacement values for each point.
       */
      Line(const T& ax, const T& ay, const T& bx, const T& by) :
         a(Point<T>(ax, ay)), b(Point<T>(bx, by)) { }
   
      /**
       * Determine if the two lines are geometrically equal, meaning
       * that they overlap.
       */
      bool operator==(const Line<T>& rhs) const {
         return a == rhs.a && b == rhs.b;
      }
      
      /**
       * Detemrine if the two lines are not geometrically equal.
       */
      bool operator!=(const Line<T>& rhs) const {
         return ! this->operator==(rhs);
      }

      /**
       * Determine if this line intersects another line.  Note that this
       * method treats each line as a continuous function rather than
       * a line segment.
       *
       * @param L the line to check against this line.
       * @param ip (optional) If specified non-null, the location to store
       *    the intersection point between this line and L if such an
       *    intersection point exists.
       * @return True if the lines intersect, false otherwise.
       */
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

      /**
       * Orient a point about the given line.  Returns <0 if
       * the point is to the left of the line, >0 if it is
       * to the right, and ~0 if it is on the line.
       */
      double orient_point(const Point<T>& pt) {
         Vector<T> V1 = to_vector();
         Vector<T> V2 = Vector<T>(pt.x - a.x, pt.y - a.y);

         return V1.cross_product(V2);
      }

      /**
       * Determine if the given point lies on the line.
       */
      bool point_on_line(const Point<T>& pt) const {
         return epsilon_equal(orient_point(pt), 0.0);
      }

      /**
       * Determine if a particular line segment intersects this line.
       *
       * NOTE:
       * - To determine if two line segments intersect, you will need to
       *   call this method twice, once with each line, e.g.:
       *
       *       if (L1.segment_intersects_line(L2) &&
       *           L2.segment_intersects_line(L1)) { ... }
       */
      bool segment_intersects_line(const Line<T>& segment) {
         return (point_on_line(segment.a) ||
                 point_on_line(segment.b) ||
                (orient_point(segment.a) < 0 ^
                 orient_point(segment.b) < 0));
      }

      /**
       * Calculate the vector between the two points of this line segment.
       */
      Vector<T> to_vector() const {
         return Vector<T>(b.x - a.x, b.y - a.y);
      }
      
      /**
       * Convert the line to a Settings object for printing.
       */
      Settings json() const {
         Settings json;
         json.set_object("a", a.json());
         json.set_object("b", b.json());
         return json;
      }

      friend ostream& operator<<(ostream& out, const Line<T>& L) {
         out << "Line" << L.json();
         return out;
      }

      Point<T> a;
      Point<T> b;
   };

   /**
    * Represents the size of an object as its width and height.
    *
    * Note that width and height elements are public.
    */
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

      /**
       * Interpret the width as a vector.
       */
      Vector<T> x_vector() const {
         return Vector<T>(width, 0);
      }

      /**
       * Interpret the height as a vector.
       */
      Vector<T> y_vector() const {
         return Vector<T>(0, height);
      }

      /**
       * Interpret the width and height as a vector.
       */
      Vector<T> xy_vector() const {
         return Vector<T>(width, height);
      }

      /**
       * Round this size to the nearest integer values for
       * width and height.  Useful for converting floating point
       * sizes to screen coordinates.
       */
      Size<int> round() const {
         return Size<int>(std::round(width), std::round(height));
      }

      /**
       * Interpret this size as a rectangle starting at the origin (0,0).
       */
      Rect<T> rect() const {
         return Rect<T>(Point<T>(), *this);
      }
      
      Settings json() const {
         Settings json;
         json.set<T>("width", width);
         json.set<T>("height", height);
         return json;
      }

      friend ostream& operator<<(ostream& out, const Size<T>& sz) {
         out << "Size" << sz.json();
         return out;
      }

      T width;
      T height;
   };
   
   /**
    * An enumeration of the sides of an axis-aligned rectangle.
    */
   enum RectSide {
      TOP      = 0,
      RIGHT    = 1,
      BOTTOM   = 2,
      LEFT     = 3,
      NONE     = 4
   };

   /**
    * Represents a rectangle with point and size components.
    *
    * Note that pt (Point) and sz (Size) elements are public.
    */
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

      /**
       * Move this rectangle to the given point.
       */
      Rect<T> move(const Point<T>& p) const {
         return Rect<T>(p, sz);
      }

      /**
       * Translate this rectangle from its current point
       * by the given vector.
       */
      Rect<T> translate(const Vector<T>& V) const {
         return move(pt + V);
      }

      /**
       * Determine if another rectangle intersects (overlaps) this
       * rectangle.
       */
      bool intersects(const Rect<T>& R2) const {
         return collide_minowski(R2) != NONE;
         /* 
         return !
            ((pt.x > R2.pt.x + R2.sz.width) ||
             (pt.x + sz.width < R2.pt.x) ||
             (pt.y > R2.pt.y + R2.sz.height) ||
             (pt.y + sz.height < R2.pt.y));
         */
      }
      
      /**
       * Determine if a particular point is contained within this
       * rectangle.
       */
      bool contains(const Point<T>& p) const {
         return p.x >= pt.x &&
                p.x <= pt.x + sz.width &&
                p.y >= pt.y &&
                p.y <= pt.y + sz.height;
      }

      /**
       * Determine if another rectangle is contained within this
       * rectangle.  R2 must be fully within this rectangle, no area
       * of it may be outside.
       */
      bool contains(const Rect<T>& R2) const {
         return (R2.pt.x >= pt.x &&
                 R2.pt.y >= pt.y &&
                (R2.sz.width + (R2.pt.x - pt.x) <= sz.width) &&
                (R2.sz.height + (R2.pt.y - pt.y) <= sz.height));
      }

      /**
       * Determine if the given line intersects any of the sides
       * of this rectangle.
       */
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

      /**
       * Round this rectangle to the nearest integer values for
       * point and size.  Useful for converting floating point
       * rectangles to screen coordinates.
       */
      Rect<int> round() const {
         return Rect<int>(pt.round(), sz.round());
      }

      /**
       * Split this rectangle into four equally sized rectangles.
       * This can be used to separate a rectangle into four quadrants,
       * in the following form:
       *
       *    I  | II
       *       |
       *    ---|---
       *    IV | III 
       *       |
       */
      vector<Rect<T>> split() const {
         vector<Rect<T>> subRects;
         Size<T> newSize = Size<T>(sz.width / 2, sz.height / 2);
         vector<Point<T>> points = {
            Point<T>(pt),
            Point<T>(pt.x + newSize.width, pt.y),
            Point<T>(pt.x + newSize.width, pt.y + newSize.height),
            Point<T>(pt.x, pt.y + newSize.height)
         };
         for (auto pt : points) {
            subRects.push_back(Rect<T>(pt, newSize));
         }
         return subRects;
      }
      
      /**
       * Return the points of the four corners of the rectangle,
       * in the following order:
       *
       *    0__1 
       *    |__|
       *    3  2 
       */
      vector<Point<T>> corners() const {
         return { 
            Point<T>(pt),
            Point<T>(pt.x + sz.width, pt.y),
            Point<T>(pt.x + sz.width, pt.y + sz.height),
            Point<T>(pt.x, pt.y + sz.height)
         };
      }
      
      /**
       * Return the center point of the rectangle.  This is the
       * average of all points along the edges, and is also 
       * known as the 'centroid' of the polygon.
       */
      Point<T> center() const {
         return pt + Vector<T>(sz.width / 2, sz.height / 2);
      }

      /**
       * Return the lines of the rectangle, in the following order:
       * 
       *       0
       *     _____
       *    |     |
       *  3 |     | 1
       *    |_____|
       *       2
       */
      vector<Line<T>> edges() const {
         vector<Point<T>> points = corners();
         return {
            Line<T>(points[0], points[1]),
            Line<T>(points[1], points[2]),
            Line<T>(points[2], points[3]),
            Line<T>(points[3], points[0])
         };
      }
      
      /**
       * Find the point at which a tile of a given size and index
       * would be contained in this rectangle.
       *
       * NOTE:
       * - No bounds checks are performed.
       * - Tiles are assumed to proceed left to right, top to bottom,
       *   e.g.:
       *
       *   0 1 2 3
       *   4 5 6 7
       *   8 9 A B
       *   C D E F
       */
      Point<T> tile_point(const Size<T>& szTile, int tileNum) const {
         int tilesPerRow = sz.width / szTile.width;
         return pt + Vector<T>(
            szTile.width * (tileNum % tilesPerRow),
            szTile.height * (tileNum / tilesPerRow));
      }

      /**
       * Find a tile rectangle of the given size and index
       * that would be contained in this rectangle.
       *
       * See tile_point() docs above for more info.
       */
      Rect<T> tile_rect(const Size<T>& szTile, int tileNum) const {
         return Rect<T>(tile_point(szTile, tileNum), szTile);
      }
      
      /**
       * Create a Polygon with the same dimensions as this rectangle.
       */
      Polygon<T> to_polygon() const {
         return Polygon<T>(corners());
      }
      
      /**
       * Determine if two rectangles are geometrically equal.
       */
      bool operator==(const Rect<T>& rhs) const {
         return pt == rhs.pt && sz == rhs.sz;
      }
      
      /**
       * Detemrine if two rectangles are not geometrically equal.
       */
      bool operator!=(const Rect<T>& rhs) const {
         return ! this->operator==(rhs);
      }
      
      /**
       * Convert this rectangle to a Settings object for printing.
       */
      Settings json() const {
         Settings json;
         json.set_object("pt", pt.json());
         json.set_object("sz", sz.json());
         return json;
      }

      /**
       * Calculate the minimum sized rectangle which will
       * contain all of the given points.
       */
      static Rect<T> minimum_bound(vector<Point<T>> points) {
         T xmin = 0, ymin = 0, xmax = 0, ymax = 0;

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

      /**
       * Calculate the minimum sized rectangle which will
       * contain all of the given rectangles.
       */
      static Rect<T> minimum_bound(vector<Rect<T>> rects) {
         vector<Point<T>> points;

         for (auto R : rects) {
            points.push_back(R.pt);
            points.push_back(R.pt + Vector<T>(R.sz.width, R.sz.height));
         }

         return minimum_bound(points);
      }
      
      /**
       * Write the rectangle to the given output stream.
       */
      friend ostream& operator<<(ostream& out, const Rect<T>& R) {
         out << "Rect" << R.json();
         return out;
      }
      
      /**
       * The top left point of the rectangle, indicates its position.
       */
      Point<T> pt;

      /**
       * The width and height of the rectangle.
       */
      Size<T> sz;
   };
   
   /**
    * Represents a shape composed of three or more points.
    */
   template<class T>
   class Polygon {
   public:
      /**
       * Create a polygon connecting the given points in order.
       */
      Polygon(const vector<Point<T>>& points) :
         pts(points)
      {
         util::assertTrue(points.size() >= 3, "Polygon must have at least three points");
      }
      
      /**
       * Get the polygon's constituent points in order.
       */
      const vector<Point<T>>& points() const {
         return pts;
      }
      
      /**
       * Get the edges (faces, sides) of the polygon in order
       * based on the order of the points.
       */
      const vector<Line<T>> edges() const {
         vector<Line<T>> edges;

         for (int x = 0; x < pts.size(); x++) {
            if (x < pts.size() - 1) {
               edges.push_back(Line<T>(pts[x], pts[x + 1]));
            } else {
               edges.push_back(Line<T>(pts[x], pts[0]));
            }
         }

         return edges;
      }
   
      /**
       * Get the edge vectors for the polygon in order.
       */
      vector<Vector<T>> edge_vectors() const {
         return alg::map(edges(), [](auto&& line) { return line.to_vector(); });
      }
      
      /**
       * Get the normals of the edge vectors in order.
       */
      vector<Vector<T>> edge_normals() const {
         return alg::map(edge_vectors(), [](auto&& vec) {
            return Vector<T>(vec.vx, -vec.ty).normalize();
         });
      }
      
      /**
       * Convert the polygon to a Settings object for printing.
       */
      Settings json() const {
         Settings json;
         json.set<T>("points", pts.size());
         json.set_object_array(alg::map(pts, [](auto&& pt) {
            return pt.json();
         }));
         return json;
      }
      
      /**
       * Write the polygon to the given output stream.
       */
      friend ostream& operator<<(ostream& out, const Polygon<T>& P) {
         ostringstream sb;
         out << "Polygon" << P.json();
         return out;
      }
      
   private:
      /**
       * The constituent points of the polygon in order.
       */
      vector<Point<T>> pts;
   };
}

