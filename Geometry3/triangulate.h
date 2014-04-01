#ifndef TRIANGULATE
#define TRIANGULATE

#include <vector>
#include <set>
#include <map>
#include <iomanip>
#include "permute.h"
#include "object.h"

using namespace std;
using namespace acp;

class Point2D : public Object {
 private:
  Parameters getParameters () { return Parameters(p); }
 protected:
  PV2 p;
 public:
  PV2 getP () { return p; }
};

typedef vector<Point2D *> Points2D;

#endif
