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

///////////////////////////////////////////////////////////////////////////////////
// Arrangement

class Edge;

class Vertex {
 public:
  Vertex () : p(0), edge(0), left(0) {}
  Vertex (Point *p) : p(p), edge(0), left(0) {}
  ~Vertex () { delete p; }

  Point *p;
  Edge *edge, *left;
  bool flag;
};

typedef vector<Vertex *> Vertices;

class Face;

class Edge {
 public:
  Edge (Vertex *tail, Edge *twin, Edge *next, bool in, bool flag)
    : tail(tail), helper(0), twin(twin), next(next), face(0), in(in), flag(flag) {}
  Vertex * head () const { return twin->tail; }
  bool incident (Edge *e) const;
  Edge * formLoop ();

  Vertex *tail, *helper;
  Edge *twin, *next;
  Face *face;
  bool in, flag;
};

typedef vector<Edge *> Edges;

class Face {
 public:
  Face () {}

  Edges boundary;
};

typedef vector<Face *> Faces;

class Arrangement {
 public:
  Arrangement () {}
  ~Arrangement ();
  Vertex * addVertex (Point *p);
  Edge * addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool in, bool flag);
  void removeEdge (Edge *e);
  void formFaces ();
  void addBoundary (Edge *e, Face *f) const;

  Vertices vertices;
  Edges edges;
  Faces faces;
};





void convexHull3 (Points &points, Points &hull);
void init (Arrangement &arr, Points &poits);
void buildTetrahedron (Arrangement &arr, Point* p1, Point* p2, Point* p3, Point* p4);
void debug (Arrangement &arr);
void outputVTK (const Points &pts, const vector<int> &data, ostream &ostr);
void pp (Point *p);
void pe (Edge *e);
void pes (Edges &edges);

#endif
