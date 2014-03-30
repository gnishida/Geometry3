#ifndef	ARRANGE
#define ARRANGE

#include "circle.h"
#include <set>
#include <map>

class Edge;

class Vertex {
 public:
  Vertex () : p(0), edge(0), left(0) {}
  Vertex (Point *p) : p(p), edge(0), left(0) {}
  ~Vertex () { delete p; }
  void addEdge (Edge *e);
  void removeEdge (Edge *e);

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
  Vector tan () const { return Vector(tail->p, head()->p); }
  bool incident (Edge *e) const;
  bool increasingX ();
  bool increasingY ();
  bool clockwise (Edge *e);
  Edge * formLoop ();
  bool outer ();

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
  Edge * addEdge (Vertex *tail = 0, Vertex *head = 0, bool flag = false);
  Edge * addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool in, bool flag);
  void removeEdge (Edge *e);
  void addLoop (const Points &pts);
  void formFaces ();
  void addBoundary (Edge *e, Face *f) const;

  Vertices vertices;
  Edges edges;
  Faces faces;
};

class HeadXOrder {
 public:
  bool operator() (Edge *e, Edge *f) const {
    return e != f && XOrder(e->head()->p, f->head()->p) == 1;
  }
};

Vertex * getVertex (Vertex *v, Vmap &vmap, Arrangement *a);

Arrangement * boundary (Arrangement *a, SetOp op);

bool boundary (Edge *e, SetOp op);

#endif
