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
class Face;
typedef vector<Face *> Faces;

class Vertex {
 public:
  Vertex () : p(0), edge(0), left(0) {}
  Vertex (Point *p) : p(p), edge(0), left(0) {}
  ~Vertex () { delete p; }
  void removeVisibleFace (Face *f);

  Point *p;
  Edge *edge, *left;
  bool flag;
  Faces visibleFaces;
};

typedef vector<Vertex *> Vertices;

class Edge {
 public:
  Edge (Vertex *tail, Edge *twin, Edge *next, bool flag)
    : tail(tail), helper(0), twin(twin), next(next), face(0), flag(flag) {}
  Vertex * head () const { return twin->tail; }
  bool incident (Edge *e) const;
  Edge * formLoop ();

  Vertex *tail, *helper;
  Edge *twin, *next;
  Face *face;
  bool flag;
};

typedef vector<Edge *> Edges;

class Face {
 public:
  Face () {}
  bool visible(Vertex *v);
  Edge* edge;
  Vertices visibleVertices;
};

class Arrangement {
 public:
  Arrangement () {}
  ~Arrangement ();
  Vertex * addVertex (Point *p);
  Edge * addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool in, bool flag);
  void removeEdge (Edge *e);
  void removeFace (Face *f);
  void formFaces ();
  void addBoundary (Edge *e, Face *f) const;

  Vertices vertices;
  Edges edges;
  Faces faces;
};





void convexHull3 (Points &points, vector<int> &hull);
void init (Arrangement &arr);
void buildTetrahedron (Arrangement &arr, Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4);
void findHorizon (Arrangement &arr, Vertex *v, Edges &horizon);
void listUpdateVertices(Arrangement &arr, Edges &horizon, Vertices &vertices);
void deleteVisibleCone(Arrangement &arr, Vertex *v);
void addCone(Arrangement &arr, Vertex *v, Edges &horizon, Vertices &vertices);
void buildHull (Arrangement &arr, vector<int> &hull);
void outputVTK (const Points &pts, const vector<int> &data, ostream &ostr);
void pp (Point *p);
void pe (Edge *e);
void pes (Edges &edges);

#endif
