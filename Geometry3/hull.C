#include "hull.h"

void convexHull (Points &points, Points &hull)
{
  for (int i = 1; i < points.size(); ++i)
    if (XOrder(points[i], points[0]) == 1) {
      Point *p = points[i];
      points[i] = points[0];
      points[0] = p;
    }
  sort(++points.begin(), points.end(), CCWOrder(points[0]));
  int nh = 0;
  for (int i = 0; i < points.size(); ++i) {
    hull.push_back(points[i]);
    ++nh;
    while (nh > 2 &&
	   LeftTurn(hull[nh-3], hull[nh-2], hull[nh-1]) == -1) {
      hull[nh-2] = hull[nh-1];
      hull.pop_back();
      --nh;
    }
  }
}


