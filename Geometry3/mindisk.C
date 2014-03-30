#include "mindisk.h"

Circle * minDisk (const Points &pts)
{
  int n = pts.size();
  if (n < 2) return 0;
  int *p = new int [n];
  randomPermutation (n, p);
  Circle *c = new Circle2pts(pts[p[0]], pts[p[1]]);
  for (int i = 2; i < pts.size(); ++i) {
    Point *r = pts[p[i]];
    if (PointInCircle(r, c) == -1) {
      delete c;
      c = minDiskWithPoint(pts, p, i, r);
    }
  }
  delete [] p;
  return c;
}

Circle * minDiskWithPoint (const Points &pts, int *p, int n, Point *q)
{
  Circle *c = new Circle2pts(pts[p[0]], q);
  for (int i = 1; i < n; ++i) {
    Point *r = pts[p[i]];
    if (PointInCircle(r, c) == -1) {
      delete c;
      c = minDiskWithTwoPoints(pts, p, i, q, r);
    }
  }
  return c;
}

Circle * minDiskWithTwoPoints (const Points &pts, int *p, int n, Point *q1,
			       Point *q2)
{
  Circle *c = new Circle2pts(q1, q2);
  for (int i = 0; i < n; ++i) {
    Point *r = pts[p[i]];
    if (PointInCircle(r, c) == -1) {
      delete c;
      c = new Circle3pts(q1, q2, r);
    }
  }
  return c;
}

