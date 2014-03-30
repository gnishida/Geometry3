#include "arrange.h"

void Vertex::addEdge (Edge *e)
{
  if (!edge) {
    edge = e;
    e->next = e;
  }
  else if (e->clockwise(edge)) {
    e->next = edge;
    Edge *f = edge;
    while (f->next != edge)
      f = f->next;
    f->next = e;
    edge = e;
  }
  else {
    Edge *f = edge;
    while (f->next != edge && f->next->clockwise(e))
      f = f->next;
    e->next = f->next;
    f->next = e;
  }
}

void Vertex::removeEdge (Edge *e)
{
  if (edge == e && e->next == e)
    edge = 0;
  else {
    Edge *f = edge;
    while (f->next != e)
      f = f->next;
    f->next = e->next;
    if (edge == e)
      edge = f;
  }
}

bool Edge::incident (Edge *e) const
{
  return tail == e->tail || tail == e->head() ||
    head() == e->tail || head() == e->head();
}

bool Edge::increasingX ()
{
  return XOrder(tail->p, head()->p) == 1;
}

bool Edge::increasingY ()
{
  return YOrder(tail->p, head()->p) == 1;
}

bool Edge::clockwise (Edge *e)
{
  bool inc = increasingY(), einc = e->increasingY();
  return inc != einc ? inc : LeftTurn(head()->p, tail->p, e->head()->p) == 1;
}

bool Edge::intersects (Edge *e)
{
  return !incident(e) &&
    LeftTurn(tail->p, e->tail->p, e->head()->p) !=
    LeftTurn(head()->p, e->tail->p, e->head()->p) &&
    LeftTurn(e->tail->p, tail->p, head()->p) !=
    LeftTurn(e->head()->p, tail->p, head()->p);
}

void Edge::intersection (Edge *e, Points &pts)
{
  if (intersects(e)) {
    pts.push_back(new LineIntersection(tail->p, head()->p,
				       e->tail->p, e->head()->p));
  }
}

Edge * Edge::formLoop ()
{
  Edge *e = this, *l = this;
  while (true) {
    e->flag = true;
    e = e->twin->next;
    if (e->flag)
      break;
    if (e->head()->p != l->head()->p &&
	XOrder(e->head()->p, l->head()->p) == 1)
      l = e;
  }
  // eps: to do: use vis version
  return l;
}

bool Edge::outer ()
{
  Edge *f = twin->next;
  return f != twin && LeftTurn(tail->p, head()->p, f->head()->p) == 1;
}

Edge * pred (Edge *e, Sweep &sweep)
{
  Sweep::iterator i = sweep.find(e);
  if (i == sweep.begin())
    return 0;
  --i;
  return *i;
}

Edge * succ (Edge *e, Sweep &sweep)
{
  Sweep::iterator i = sweep.find(e);
  ++i;
  return i == sweep.end() ? 0 : *i;
}

bool Event::operator< (Event &e)
{ 
  if (type == Insert && e.type == Remove && a->head() == e.a->tail)
    return true;
  if (type == Remove && e.type == Insert && a->tail == e.a->head())
    return false;
  if (type == Insert && e.type == Insert && a->head() == e.a->head() ||
      type == Remove && e.type == Remove && a->tail == e.a->tail)
    return a < e.a;
  return YOrder(p, e.p) == 1;
}

void makeHeap (const Edges &edges, Events &heap)
{
  for (Edges::const_iterator e = edges.begin(); e != edges.end(); ++e)
    if ((*e)->increasingY())
      heap.push_back(Event(Remove, (*e)->tail->p, *e));
    else
      heap.push_back(Event(Insert, (*e)->tail->p, (*e)->twin));
  make_heap(heap.begin(), heap.end());
}

Event nextEvent (Events &heap)
{
  Event e = heap[0];
  do {
    pop_heap(heap.begin(), heap.end());
    heap.pop_back();
  }
  while (!heap.empty() && e.type == Swap && heap[0].type == Swap &&
	 e.a == heap[0].a && e.b == heap[0].b);
  return e;
}

void pushHeap (const Event &e, Events &heap)
{
  heap.push_back(e);
  push_heap(heap.begin(), heap.end());
}

Event popHeap (Events &heap)
{
  Event e = heap[0];
  pop_heap(heap.begin(), heap.end());
  heap.pop_back();
  return e;
}

Arrangement::~Arrangement ()
{
  for (Vertices::iterator v = vertices.begin(); v != vertices.end(); ++v)
    delete *v;
  for (Edges::iterator e = edges.begin(); e != edges.end(); ++e)
    delete *e;
  for (Faces::iterator f = faces.begin(); f != faces.end(); ++f)
    delete *f;
  for (Circles::iterator c = circles.begin(); c != circles.end(); ++c)
    delete *c;
}

Vertex * Arrangement::addVertex (Point *p)
{
  Vertex *v = new Vertex(p);
  vertices.push_back(v);
  return v;
}

Edge * Arrangement::addEdge (Vertex *tail, Vertex *head, bool aflag, bool flag)
{
  Edge *e = addHalfEdge(tail, 0, 0, true, aflag, flag),
    *et = addHalfEdge(head, e, 0, false, aflag, flag);
  e->twin = et;
  tail->addEdge(e);
  head->addEdge(et);
  return e;
}

Edge * Arrangement::addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool in,
				 bool aflag, bool flag)
{
  Edge *e = new Edge(tail, twin, next, in, aflag, flag);
  edges.push_back(e);
  return e;
}

void Arrangement::removeEdge (Edge *e)
{
  int c = 0, i = 0;
  while (c < 2 && i < edges.size())
    if (edges[i] == e || edges[i] == e->twin) {
      edges[i] = *(edges.end()-1);
      edges.pop_back();
      ++c;
    }
    else
      ++i;
  e->tail->removeEdge(e);
  e->twin->tail->removeEdge(e->twin);
  delete e->twin;
  delete e;
}

void Arrangement::addLoop (const Points &pts)
{
  int n = pts.size();
  Vertex *t = addVertex(pts[0]->copy()), *t0 = t;
  for (int i = 1; i <= n; ++i) {
    Vertex *h = i < n ? addVertex(pts[i]->copy()) : t0;
    addEdge(t, h);
    t = h;
  }
}

void Arrangement::intersectEdges ()
{
  Events heap;
  makeHeap(edges, heap);
  Sweep sweep;
  EpairSet eset;
  while (!heap.empty()) {
    Event e = popHeap(heap);
    switch (e.type) {
    case Insert:
      insert(e.a, sweep, heap, eset);
      break;
    case Remove:
      remove(e.a, sweep, heap, eset);
      break;
    case Swap:
      swap(e.a, e.b, e.p, sweep, heap, eset);
    }
  }
}

void Arrangement::insert (Edge *e, Sweep &sweep, Events &heap,
			  EpairSet &eset) const
{
  sweep.insert(e);
  Edge *p = pred(e, sweep), *s = succ(e, sweep);
  if (p && !e->head()->left && e->head() != p->head())
    e->head()->left = p->twin;
  check(p, e, heap, eset);
  check(e, s, heap, eset);
}

void Arrangement::remove (Edge *e, Sweep &sweep, Events &heap,
			  EpairSet &eset) const
{
  Edge *p = pred(e, sweep), *s = succ(e, sweep);
  sweep.erase(e);
  if (p && p->tail != e->tail)
    e->tail->left = p->twin;
  check(p, s, heap, eset);
}

void Arrangement::swap (Edge *e, Edge *f, Point *a, Sweep &sweep,
			Events &heap, EpairSet &eset)
{
  Edge *p = pred(e, sweep), *s = succ(f, sweep);
  sweep.erase(e);
  sweep.erase(f);
  split(e, f, a);
  sweep.insert(e);
  sweep.insert(f);
  if (p)
    e->head()->left = p->twin;
  check(p, f, heap, eset);
  check(e, s, heap, eset);
}

void Arrangement::check (Edge *e, Edge *f, Events &heap, EpairSet &eset) const
{
  if (!e || !f || rbflag && e->aflag == f->aflag)
    return;
  Epair ef(e < f ? e : f, e < f ? f : e);
  if (eset.find(ef) != eset.end())
    return;
  eset.insert(ef);
  Points pts;
  e->intersection(f, pts);
  for (Points::iterator p = pts.begin(); p != pts.end(); ++p) {
    pushHeap(Event(Swap, *p, e, f), heap);
    Edge *t = e;
    e = f;
    f = t;
  }
}

void Arrangement::split (Edge *e, Edge *f, Point *p)
{
  Vertex *v = addVertex(p);
  Edge *et = e->twin, *ft = f->twin, 
    *e4 = addHalfEdge(v, et, 0, e->in, e->aflag, e->flag),
    *e3 = addHalfEdge(v, f, e4, ft->in, ft->aflag, ft->flag), 
    *e2 = addHalfEdge(v, e, e3, et->in, et->aflag, et->flag),
    *e1 = addHalfEdge(v, ft, e2, f->in, f->aflag, f->flag);
  e4->next = e1;
  e->twin = e2;
  et->twin = e4;
  f->twin = e3;
  ft->twin = e1;
  v->edge = e1;
}

void Arrangement::formFaces ()
{
  for (Faces::iterator f = faces.begin(); f != faces.end(); ++f)
    delete *f;
  faces.clear();
  Edges inner;
  faces.push_back(new Face);
  for (Edges::iterator e = edges.begin(); e != edges.end(); ++e)
    if (!(*e)->flag) {
      Edge *l = (*e)->formLoop();
      if (l->outer()) {
	Face *f = new Face;
	faces.push_back(f);
	addBoundary(l, f);
      }
      else
	inner.push_back(l);
    }
  sort(inner.begin(), inner.end(), HeadXOrder());
  for (Edges::iterator e = inner.begin(); e != inner.end(); ++e) {
    Vertex *v = (*e)->head();
    Face *f = v->left ? v->left->face : faces[0];
    addBoundary(*e, f);
  }
}

void Arrangement::addBoundary (Edge *e, Face *f) const
{
  f->boundary.push_back(e);
  Edge *g = e;
  do {
    g->tail->flag = true;
    g->flag = true;
    g->face = f;
    g = g->twin->next;
  }
  while (g != e);
}

void Arrangement::computeWindingNumbers () const
{
  Faces st;
  st.push_back(faces[0]);
  faces[0]->wna = faces[0]->wnb = 0;
  while (!st.empty()) {
    Face *f = *(st.end()-1);
    st.pop_back();
    for (Edges::iterator b = f->boundary.begin(); 
	 b != f->boundary.end(); ++b) {
      Edge *e = *b;
      do {
	Face *g = e->twin->face;
	if (g->wna == -1) {
	  if (e->aflag) {
	    g->wna = e->in ? f->wna - 1 : f->wna + 1;
	    g->wnb = f->wnb;
	  }
	  else {
	    g->wna = f->wna;
	    g->wnb = e->in ? f->wnb - 1 : f->wnb + 1;
	  }
	  st.push_back(g);
	}
	e = e->twin->next;
      }
      while (e != *b);
    }
  }
}

Arrangement * setOperation (Arrangement *a, Arrangement *b, SetOp op)
{
  Arrangement *o = overlay(a, b);
  Arrangement *s = boundary(o, op);
  delete o;
  return s;
}

Arrangement * overlay (Arrangement *a, Arrangement *b)
{
  Arrangement *arr = new Arrangement(true);
  copyEdges(a, true, arr);
  copyEdges(b, false, arr);
  arr->intersectEdges();
  arr->formFaces();
  arr->computeWindingNumbers();
  return arr;
}

void copyEdges (Arrangement *a, bool aflag, Arrangement *b)
{
  Vmap vmap;
  for (Edges::iterator e = a->edges.begin(); e != a->edges.end(); ++e)
    if ((*e)->in)
      copyEdge(*e, aflag, vmap, b);
}

void copyEdge (Edge *e, bool aflag, Vmap &vmap, Arrangement *a)
{
  Vertex *t = getVertex(e->tail, vmap, a),
    *h = getVertex(e->head(), vmap, a);
  a->addEdge(t, h, aflag);
}

Vertex * getVertex (Vertex *v, Vmap &vmap, Arrangement *a)
{
  Vmap::iterator i = vmap.find(v);
  if (i != vmap.end())
    return i->second;
  Vertex *w = a->addVertex(v->p->copy());
  vmap.insert(Vpair(v, w));
  return w;
}

Arrangement * boundary (Arrangement *a, SetOp op)
{
  Vmap vmap;
  Arrangement *b = new Arrangement;
  for (Edges::iterator e = a->edges.begin(); e != a->edges.end(); ++e) 
    if (boundary(*e, op))
      copyEdge(*e, true, vmap, b);
  b->formFaces();
  return b;
}

bool boundary (Edge *e, SetOp op)
{
  bool ein = setop(op, e->face->wna, e->face->wnb),
    etin = setop(op, e->twin->face->wna, e->twin->face->wnb);
  return ein && !etin;
}

bool setop (SetOp op, int wna, int wnb)
{
  bool ina = wna%2 == 1, inb = wnb%2 == 1;
  switch (op) {
  case Union: 
    return ina || inb;
  case Intersection: 
    return ina && inb;
  case AminusB:
    return ina && !inb;
  case Minkowski:
    return wna > 0;
  }
  return false;
}

void pe (Edge *e)
{
  cerr << "(";
  pp(e->tail->p);
  pp(e->head()->p);
  cerr << ")";
}

void pes (Edges &edges)
{
  cerr << "(";
  for (int i = 0; i < edges.size(); ++i)
    pe(edges[i]);
  cerr << ")" << endl;
}

