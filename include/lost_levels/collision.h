/*
 * collision: Collision detection algorithms and helpers.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Thursday, Dec 22 2016
 */
#pragma once
#include "geometry.h"

namespace lost_levels { 
   template<class T>
   class Collider {
   public:
      /**
       * Determine the direction of collision if two rectangles
       * overlap by calculating the Minowski sum of the two
       * rectangles.  If the rectangles do not overlap,
       * RectSide.NONE is returned (check for it!)
       */
      RectSide collide(const Rect<T>& r1, const Rect<T>& r2) {
         Point<T> centerA = r1.center(),
                  centerB = r2.center();
         float w  = (r1.sz.width + r2.sz.width) / ((T) 2),
               h  = (r1.sz.height + r2.sz.height) / ((T) 2),
               dx = centerA.x - centerB.x,
               dy = centerA.y - centerB.y;

         if (abs(dx) <= w && abs(dy) <= h) {
            float wy = w * dy;
            float hx = h * dx;
            
            if (wy > hx) {
               return wy > -hx ? TOP : LEFT;
            } else {
               return wy > -hx ? RIGHT : BOTTOM;
            }
         } else {
            return NONE;
         }
      }
   };

   template<class T>
   class Projection {
   public:
      Projection(T min, T max) :
         min(min), max(max)
      { }

      static Projection<T> ofPolygonOnAxis(const Polygon<T>& polygon, const Vector<T>& axis) {
         Projection proj(0, 0);

         for (auto pt : polygon.points()) {
            T dp = pt.to_vector().dot_product(axis);
            if (dp < proj.min) {
               proj.min = dp;
            } else if (dp > proj.max) {
               proj.max = dp;
            }
         }

         return proj;
      }

      friend T operator-(const Projection& projA, const Projection& projB) {
         if (projA.min < projB.min) {
            return projB.min - projA.max;
         } else {
            return projA.min - projB.max;
         }
      }

      T min, max;
   };
   
   template<class T>
   class CollisionResult {
   public:
      bool will_intersect;
      bool are_intersecting;
      Vector<T> min_trans_v;
   };

   /**
    * Divides the given fixed size cartesian plane into subdivisions to
    * optimize the targets for collision detection.
    */
   template<class T, class C>
   class CollisionTree {
   public:
      CollisionTree(const Rect<T>& rect, int level = 0,
               int maxLevel = 5, int maxObjects = 10)
         : rect(rect), level(level), maxLevel(maxLevel), maxObjects(maxObjects)
      { }

      typedef pair<C, Rect<T>> Entry;

      virtual ~CollisionTree()
      { }
      
      void insert(const C& object, const Rect<T>& rect) {
         insert({object, rect});
      }

      void clear() {
         entries.clear();
         quadrants.clear();
      }
      
      vector<Entry> retrieve(const Rect<T>& objRect) const {
         vector<Entry> potentials;
         retrieve_impl(potentials, objRect);
         return potentials;
      }

      const Rect<T>& get_rect() const {
         return rect;
      }

      vector<const CollisionTree<T, C>*> get_quadrants() const {
         vector<const CollisionTree<T, C>*> quadPointers;
         for (int x = 0; x < quadrants.size(); x++) {
            quadPointers.push_back(&quadrants[x]);
         }
         return quadPointers;
      }

      void debug_split() {
         split();
      }

      Settings json() const {
         Settings json;
         
         vector<Settings> quadListJson;
         for (auto quad : quadrants) {
            quadListJson.push_back(quad.json());
         }
         json.set_object_array("quadrants", quadListJson);

         vector<Settings> entryRectListJson;
         for (auto entry : entries) {
            entryRectListJson.push_back(entry.second.json());
         }
         json.set_object_array("entries", entryRectListJson);
         json.set_object("rect", rect.json());

         return json;
      }

   protected:
      int get_index(const Rect<T>& objRect) const {
         int idx = -1;

         for (int x = 0; x < quadrants.size(); x++) {
            if (quadrants.at(x).rect.contains(objRect)) {
               idx = x;
               break;
            }
         }
         
         return idx;
      }

      void insert(const Entry& entry) {
         if (quadrants.size() > 0) {
            int idx = get_index(entry.second);
            if (idx != -1) {
               quadrants.at(idx).insert(entry);
               return;
            }
         }

         entries.push_back(entry);
         if (entries.size() >= maxObjects && level < maxLevel) {
            split();   
         }
      }
      
      void retrieve_impl(vector<Entry>& potentials, const Rect<T>& objRect) const {
         int idx = get_index(objRect);
         if (idx != -1 && quadrants.size() > 0) {
            quadrants.at(idx).retrieve_impl(potentials, objRect);
         }
         
         potentials.insert(potentials.end(), entries.begin(), entries.end());
      }

      void split() {
         if (quadrants.size() > 0) {
            return;
         }

         vector<Rect<T>> subRects = rect.split();
         for (auto subRect : subRects) {
            quadrants.push_back(CollisionTree(subRect, level + 1, maxLevel, maxObjects));
         }

         vector<Entry> savedEntries = entries;
         entries.clear();

         for (auto entry : savedEntries) {
            insert(entry);
         }
      }

   private:
      int level;
      int maxLevel;
      int maxObjects;
      Rect<T> rect;
      vector<Entry> entries;
      vector<CollisionTree<T, C>> quadrants;
   }; 
}
