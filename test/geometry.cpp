#include "lost_levels/geometry.h"
#include "lain/testing.h"

using namespace std;
using namespace lain;
using namespace lain::testing;
using namespace lost_levels;

template <class T>
bool vector_point_test(T x, T y, T vx, T vy) {
   Vector<T> V = Vector<T>(vx, vy);
   Point<T> p1 = Point<T>(x, y);
   Point<T> p2 = p1 + V;

   cout << p1 << " + " << V
        << " = " << p2 << endl;

   assert_equal(p2.x, p1.x + V.vx);
   assert_equal(p2.y, p1.y + V.vy);

   p2 = p1 - V;

   cout << p1 << " - " << V
        << " = " << p2 << endl;

   assert_equal(p2.x, p1.x - V.vx);
   assert_equal(p2.y, p1.y - V.vy);

   return true;
}

template <class T>
bool vector_test(T vx, T vy, T s) {
   Vector<T> A = Vector<T>(vx, vy);
   Vector<T> B = A * s;
   Vector<T> C = A + B;

   Vector<int> R = C.round();

   cout << A << " * " << s
        << " = " << B << endl;

   assert_equal(B.vx, A.vx * s);
   assert_equal(B.vy, A.vy * s);

   cout << A << " + " << B
        << " = " << C << endl;

   assert_equal(C.vx, A.vx + B.vx);
   assert_equal(C.vy, A.vy + B.vy);

   cout << C << ".round() = "
        << R << endl;

   assert_equal<int>(round(C.vx), R.vx);
   assert_equal<int>(round(C.vy), R.vy);

   return true;
}

void rect_intersect_test(const Rect<int>& R1, const Rect<int>& R2, bool result) {
   cout << R1 << string(result ? " intersects " : " does not intersect ") << R2 << endl;
   assert_true(R1.intersects(R2) == result);
}

void rect_point_contains_test(const Rect<int>& R1, const Point<int>& P, bool result) {
   cout << R1 << string(result ? " contains " : " does not contain ") << P << endl;
   assert_true(R1.contains(P) == result);
}

void rect_line_intersect_test(const Rect<int>& R1, const Line<int>& L, bool result) {
   cout << R1 << string(result ? " intersects " : " does not intersect ") << L << endl;
   assert_true(R1.intersects(L) == result);
}

void rect_rect_contains_test(const Rect<int>& R1, const Rect<int>& R2, bool result) {
   cout << R1 << string(result ? " contains " : " does not contain ") << R2 << endl;
   assert_true(R1.contains(R2) == result);
}

int randi(int imin = -100, int imax = 100) {
   return rand() % (imax - imin) + imin;
}

double randf(double fmin = -100.0, double fmax = 100.0) {
   double f = (double)(rand()) / RAND_MAX;
   return fmin + f * (fmax - fmin);
}

int main() {
   srand(time(0));

   return TestSuite("lost_levels geometry tests")
      .die_on_signal(SIGSEGV)
      .test("Geometry-001: Point and Vector Arithmetic", [&]()->bool {
         return
            vector_point_test<int>(randi(), randi(), randi(), randi()) &&
            vector_point_test<double>(randf(), randf(), randf(), randf()) &&
            vector_point_test<float>(randf(), randf(), randf(), randf());
      })
      .test("Geometry-002: Vector Arithmetic", [&]()->bool {
         return
            vector_test<int>(randi(), randi(), randi()) &&
            vector_test<double>(randf(), randf(), randf()) &&
            vector_test<double>(randf(), randf(), randf());
      })
      .test("Geometry-003: Rectangle minimum bound", [&]()->bool {
         vector<Rect<int>> rectangles;

         for (int x = 0; x < 5; x++) {
            Rect<int> R = Rect<int>(x * 10, x * 10, 50, 50);
            rectangles.push_back(R);
            cout << R << endl;
         }

         Rect<int> minBound = Rect<int>::minimum_bound(rectangles);

         cout << "Minimum bound: " << minBound << endl;
         assert_equal(minBound, Rect<int>(0, 0, 90, 90));

         return true;
      })
      .test("Geometry-004: Rectangle intersection", [&]()->bool {
         Rect<int> R1 = Rect<int>(0, 0, 10, 10);
         Rect<int> R2 = Rect<int>(10, 10, 10, 10);
         Rect<int> R3 = Rect<int>(-10, -10, 5, 5);
         Rect<int> R4 = Rect<int>(11, 11, 8, 8);

         Point<int> P1 = Point<int>(1, 1);
         Point<int> P2 = Point<int>(-1, -1);

         Line<int> L1 = Line<int>(-1, -1, 11, 11);
         Line<int> L2 = Line<int>(0, -11, 5, 12);
         Line<int> L3 = Line<int>(1, 1, 9, 9);
         Line<int> L4 = Line<int>(-1, -1, -11, -11);
         Line<int> L5 = Line<int>(1, 0, 1, 100);

         rect_intersect_test(R1, R2, true);
         rect_intersect_test(R2, R1, true);
         rect_intersect_test(R1, R3, false);
         rect_intersect_test(R3, R1, false);
         rect_intersect_test(R2, R3, false);
         rect_intersect_test(R3, R2, false);
         rect_intersect_test(R2, R4, true);
         rect_intersect_test(R4, R2, true);

         rect_point_contains_test(R1, P1, true);
         rect_point_contains_test(R1, P2, false);

         rect_line_intersect_test(R1, L1, true);
         rect_line_intersect_test(R1, L2, true);
         rect_line_intersect_test(R1, L3, true);
         rect_line_intersect_test(R1, L4, false);
         rect_line_intersect_test(R1, L5, true);

         return true;
      })
      .test("Geometry-005: Rectangle contains other rectangles", [&]()->bool {
         Rect<int> R0 = Rect<int>(0, 0, 10, 10);
         Rect<int> R1 = Rect<int>(0, 0, 5, 5);
         Rect<int> R2 = Rect<int>(5, 0, 5, 5);
         Rect<int> R3 = Rect<int>(0, 5, 5, 5);
         Rect<int> R4 = Rect<int>(5, 5, 5, 5);
         Rect<int> R5 = Rect<int>(2, 2, 5, 5);
         Rect<int> R6 = Rect<int>(7, 7, 5, 5);
         Rect<int> R7 = Rect<int>(-10, -10, 5, 5);

         rect_rect_contains_test(R0, R1, true);
         rect_rect_contains_test(R0, R2, true);
         rect_rect_contains_test(R0, R3, true);
         rect_rect_contains_test(R0, R4, true);
         rect_rect_contains_test(R0, R5, true);
         rect_rect_contains_test(R0, R6, false);
         rect_rect_contains_test(R0, R7, false);;

         return true;
      })

      .run();
}

