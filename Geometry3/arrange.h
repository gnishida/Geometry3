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
  Edge (Vertex *tail, Edge *twin, Edge *next, bool in, bool aflag, bool flag)
    : tail(tail), helper(0), twin(twin), next(next), face(0), in(in), 
    aflag(aflag), flag(flag) {}
  Vertex * head () const { return twin->tail; }
  Vector tan () const { return Vector(tail->p, head()->p); }
  bool incident (Edge *e) const;
  bool increasingX ();
  bool increasingY ();
  bool clockwise (Edge *e);
  bool intersects (Edge *e);
  void intersection (Edge *e, Points &pts);
  Edge * formLoop ();
  bool outer ();

  Vertex *tail, *helper;
  Edge *twin, *next;
  Face *face;
  bool in, aflag, flag;
};

typedef vector<Edge *> Edges;

class Epair {
 public:
  Epair (Edge *e, Edge *f) : e(e), f(f) {}
  bool operator< (const Epair &p) const {
    return e < p.e || e == p.e && f < p.f;
  }

  Edge *e, *f;
};

typedef set<Epair> EpairSet;

class Face {
 public:
  Face () : wna(-1), wnb(-1) {}

  Edges boundary;
  int wna, wnb;
};

typedef vector<Face *> Faces;

class EdgeXOrder {
 public:
  bool operator() (Edge *e, Edge *f) const {
    if (e == f)
      return false;
    if (e->head() == f->head())
      return LeftTurn(f->tail->p, f->head()->p, e->tail->p) == 1;
    return YOrder(e->head()->p, f->head()->p) == 1 ?
      LeftTurn(f->tail->p, f->head()->p, e->head()->p) == 1 :
      LeftTurn(e->tail->p, e->head()->p, f->head()->p) == -1;
  }
};

typedef set<Edge *, EdgeXOrder> Sweep;

Edge * pred (Edge *e, Sweep &sweep);

Edge * succ (Edge *e, Sweep &sweep);

enum EventType {Insert, Remove, Swap};

class Event {
 public:
  Event (EventType type, Point *p, Edge *a, Edge *b = 0) 
    : type(type), p(p), a(a), b(b) {}
  bool operator< (Event &x);
  
  EventType type;
  Point *p;
  Edge *a, *b;
};

typedef vector<Event> Events;

void makeHeap (const Edges &edges, Events &heap);

Event nextEvent (Events &heap);

void pushHeap (const Event &e, Events &heap);

Event popHeap (Events &heap);

class Arrangement {
 public:
  Arrangement (bool rbflag = false) :rbflag(rbflag) {}
  ~Arrangement ();
  Vertex * addVertex (Point *p);
  Edge * addEdge (Vertex *tail = 0, Vertex *head = 0, bool aflag = true,
		  bool flag = false);
  Edge * addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool in,
		      bool aflag, bool flag);
  void removeEdge (Edge *e);
  void addLoop (const Points &pts);
  void intersectEdges ();
  void insert (Edge *e, Sweep &sweep, Events &heap, EpairSet &eset) const;
  void remove (Edge *e, Sweep &sweep, Events &heap, EpairSet &eset) const;
  void swap (Edge *e, Edge *f, Point *a, Sweep &sweep, 
	     Events &heap, EpairSet &eset);
  void check (Edge *e, Edge *f, Events &heap, EpairSet &eset) const;
  void split (Edge *e, Edge *f, Point *p);
  void formFaces ();
  void addBoundary (Edge *e, Face *f) const;
  void computeWindingNumbers () const;

  Vertices vertices;
  Edges edges;
  Faces faces;
  bool rbflag;
  Circles circles;
};

class HeadXOrder {
 public:
  bool operator() (Edge *e, Edge *f) const {
    return e != f && XOrder(e->head()->p, f->head()->p) == 1;
  }
};

typedef pair<Vertex *, Vertex *> Vpair;

typedef map<Vertex *, Vertex *> Vmap;

enum SetOp {Union, Intersection, AminusB, Minkowski};

Arrangement * setOperation (Arrangement *a, Arrangement *b, SetOp op);

Arrangement * overlay (Arrangement *a, Arrangement *b);

void copyEdges (Arrangement *a, bool aflag, Arrangement *b);

void copyEdge (Edge *e, bool aflag, Vmap &vmap, Arrangement *a);

Vertex * getVertex (Vertex *v, Vmap &vmap, Arrangement *a);

Arrangement * boundary (Arrangement *a, SetOp op);

bool boundary (Edge *e, SetOp op);

bool setop (SetOp op, int wna, int wnb);

#endif
