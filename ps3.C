#ifndef HULL3
#define HULL3

#include <vector>
#include <set>
#include <map>
#include <iomanip>
#include "permute.h"
#include "object.h"

using namespace std;
using namespace acp;

class Point : public Object {
 private:
  Parameters getParameters () { return Parameters(p); }
 protected:
  PV3 p;
 public:
  PV3 getP () { return p; }
};

typedef vector<Point *> Points;

Predicate4(Orient3D, Point *, a, Point *, b, Point *, c, Point *,d);

class InputPoint : public Point {
 private:
  Objects getObjects () { return Objects(); }
  void calculate () {}
  InputPoint (const PV3 &ip) { p = ip; }
 public:
  InputPoint (double x, double y, double z) { p = PV3(x, y, z); }
};

int Orient3D::sign ()
{
  PV3 u = d->getP() - a->getP(), v = b->getP() - a->getP(), 
    w = c->getP() - a->getP();
  return u.tripleProduct(v, w).sign();
}

void outputVTK (const Points &pts, const vector<int> &data, ostream &ostr)
{
  int np = pts.size(), nt = data.size()/3;
  ostr << setprecision(16) << "# vtk DataFile Version 3.0" << endl
       << "vtk output" << endl << "ASCII" << endl
       << "DATASET POLYDATA" << endl 
       << "POINTS " << np << " double" << endl;
  for (Points::const_iterator p = pts.begin(); p != pts.end(); ++p) {
    const PV3 &q = (*p)->getP();
    ostr << q.x.mid() << " " << q.y.mid() << " " << q.z.mid() << endl;
  }
  ostr << endl << "POLYGONS " << nt << " " << 4*nt << endl;
  for (int i = 0; i < nt; ++i)
    ostr << "3 " << data[3*i] << " " << data[3*i+1] << " "
	 << data[3*i+2] << endl;
  ostr << endl << "CELL_DATA " << nt << endl 
       << "POINT_DATA " << np << endl;
}

