#ifndef HULL
#define HULL

#include "point.h"

class CCWOrder {
 public:
  CCWOrder (Point *o) : o(o) {}
  bool operator() (Point *a, Point *b) const {
    return LeftTurn(a, o, b) == -1;
  }

 private:
  Point *o;
};

void convexHull (Points &points, Points &hull);

#endif
