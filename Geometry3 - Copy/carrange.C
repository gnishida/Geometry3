#include "carrange.h"

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

bool Edge::incident (Edge *e) const
{
  return tail == e->tail || tail == e->head() ||
    head() == e->tail || head() == e->head();
}

bool Edge::increasingX ()
{
  XOrder(tail->p, head()->p) == 1;
}

bool Edge::increasingY ()
{
  return YOrder(tail->p, head()->p) == 1;
}

bool Edge::clockwise (Edge *e)
{
  CircleTurningPoint *v = dynamic_cast<CircleTurningPoint *>(tail->p);
  if (v)
    return v->getT() < 3 ? ccw : !ccw;
  bool inc = increasingY(), einc = e->increasingY();
  if (inc != einc)
    return inc;
  CircleTangent u(circle, tail->p, ccw), eu(e->circle, e->tail->p, e->ccw);
  return CCW(&eu, &u) == 1;
}

// both edges decreasing in Y
bool Edge::leftOf (Edge *e)
{
  if (tail == e->tail) {
    CircleTurningPoint *v = dynamic_cast<CircleTurningPoint *>(tail->p);
    return v ? ccw : !clockwise(e);
  }
  if (e->ccw)
    return LeftTurn(e->tail->p, e->head()->p, tail->p) == -1 &&
      PointInCircle(tail->p, e->circle) == -1;
  return LeftTurn(e->tail->p, e->head()->p, tail->p) == -1 ||
    PointInCircle(tail->p, e->circle) == 1;
}

void Edge::intersection (Edge *e, Points &pts)
{
  Circle *c = circle, *d = e->circle;
  if (c == d || IntersectingCircles(c, d) == -1)
    return;
  Point *pef = onCircle(e->tail) ? e->tail->p : 0;
  if (onCircle(e->head())) {
    if (pef)
      return;
    pef = e->head()->p;
  }
  if (e->onCircle(tail)) {
    if (pef)
      return;
    pef = tail->p;
  }
  if (e->onCircle(head())) {
    if (pef)
      return;
    pef = head()->p;
  }
  int s = pef ? IntersectionSign(c, d, pef) : 0;
  CircleIntersection *q1 = new CircleIntersection(c, d, true),
    *q2 = new CircleIntersection(c, d, false);
  if (s && s == IntersectionSign(c, d, q1) ||
      !(containsCirclePoint(q1) && e->containsCirclePoint(q1))) {
    delete q1;
    q1 = 0;
  }
  if (s && s == IntersectionSign(c, d, q2) ||
      !(containsCirclePoint(q2) && e->containsCirclePoint(q2))) {
    delete q2;
    q2 = 0;
  }
  if (q1 && q2 && YOrder(q1, q2) == -1) {
    pts.push_back(q1);
    pts.push_back(q2);
  }
  else if (q1 && q2) {
    pts.push_back(q2);
    pts.push_back(q1);
  }
  else if (q1)
    pts.push_back(q1);
  else if (q2)
    pts.push_back(q2);
}

bool Edge::onCircle (Vertex *v) const
{
  if (v == tail || v == head())
    return true;
  CircleIntersection *p = dynamic_cast<CircleIntersection *>(v->p);
  return p && (p->getC() == circle || p->getD() == circle);
}

bool Edge::containsCirclePoint (Point *p)
{
  int s = ccw ? 1 : -1;
  CircleCenter o(circle);
  return LeftTurn(p, &o, tail->p) == s && LeftTurn(head()->p, &o, p) == s;
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
  return l;
}

bool Edge::outer ()
{
  Edge *f = twin->next;
  return f != twin && twin->clockwise(f);
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
  if (a->circle == e.a->circle) {
    CircleTurningPoint *ct1 = 0, *ct2 = 0;
    if (type == Insert)
      ct1 = dynamic_cast<CircleTurningPoint *>(a->head()->p);
    else if (type == Remove)
      ct1 = dynamic_cast<CircleTurningPoint *>(a->tail->p);
    if (e.type == Insert)
      ct2 = dynamic_cast<CircleTurningPoint *>(e.a->head()->p);
    else if (e.type == Remove)
      ct2 = dynamic_cast<CircleTurningPoint *>(e.a->tail->p);
    if (ct1 && ct2) {
      int t1 = ct1->getT(), t2 = ct2->getT();
      if (t1 == 1 && t2 == 3)
	return true;
      if (t1 == 3 && t2 == 1)
	return false;
    }
  }
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
  for (Circles::iterator c = circles.begin(); c != circles.end(); ++c)
    delete *c;
  for (Vertices::iterator v = vertices.begin(); v != vertices.end(); ++v)
    delete *v;
  for (Edges::iterator e = edges.begin(); e != edges.end(); ++e)
    delete *e;
  for (Faces::iterator f = faces.begin(); f != faces.end(); ++f)
    delete *f;
}

Vertex * Arrangement::addVertex (Point *p)
{
  Vertex *v = new Vertex(p);
  vertices.push_back(v);
  return v;
}

Edge * Arrangement::addEdge (Vertex *tail, Vertex *head, bool in, bool aflag,
			     bool ccw, Circle *circle)
{
  Edge *e = addHalfEdge(tail, 0, 0, in, aflag, ccw, circle),
    *et = addHalfEdge(head, e, 0, !in, aflag, !ccw, circle);
  e->twin = et;
  tail->addEdge(e);
  head->addEdge(et);
  return e;
}

Edge * Arrangement::addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool in,
				 bool aflag, bool ccw, Circle *circle)
{
  Edge *e = new Edge(tail, twin, next, in, ccw, aflag, circle);
  edges.push_back(e);
  return e;
}

void Arrangement::addArcLoop (const Points &pts)
{
  int n = pts.size();
  Vertex *t = addVertex(pts[0]->copy()), *t0 = t;
  for (int i = 1; i < n; i += 2) {
    Point *m = pts[i]->copy();
    Vertex *h = i + 1 == n ? t0 : addVertex(pts[i+1]->copy());
    addArc(t, m, h);
    t = h;
  }
}

void Arrangement::addArc (Vertex *t, Point *m, Vertex *h)
{
  Circle *c = new Circle3pts(t->p, m, h->p);
  circles.push_back(c);
  bool ccw = LeftTurn(t->p, m, h->p) == 1;
  Vertex *s = ccw ? t : h, *e = ccw ? h : t;
  Vertices v;
  splitArc(c, s, e, v);
  for (int i = 0; i + 1 < v.size(); ++i)
    addEdge(v[i], v[i+1], ccw, true, true, c);
}

void Arrangement::splitArc (Circle *c, Vertex *s, Vertex *e, Vertices &v)
{
  int qs = quadrant(c, s->p), qe = quadrant(c, e->p);
  v.push_back(s);
  while (qs != qe) {
    qs = qs == 4 ? 1 : qs + 1;
    Vertex *w = addVertex(new CircleTurningPoint(c, qs));
    v.push_back(w);
  }
  v.push_back(e);
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
    *e4 = addHalfEdge(v, et, 0, e->in, e->aflag, e->ccw, e->circle),
    *e3 = addHalfEdge(v, f, e4, ft->in, ft->aflag, ft->ccw, ft->circle), 
    *e2 = addHalfEdge(v, e, e3, et->in, et->aflag, et->ccw, et->circle),
    *e1 = addHalfEdge(v, ft, e2, f->in, f->aflag, f->ccw, f->circle);
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
  a->addEdge(t, h, e->in, aflag, e->ccw, e->circle);
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

// debug

void discretize (const PV2 &o, const Parameter &r, const PV2 &t, const PV2 &h,
		 double eps, vector<PV2> &pts)
{
  vector<PV2> st;
  st.push_back(h);
  pts.push_back(t);
  while (!st.empty()) {
    PV2 p = *(st.end()-1), q = *(pts.end()-1);
    if ((p - q).dot(p - q).ub() < eps*eps) {
      pts.push_back(p);
      st.pop_back();
    }
    else {
      PV2 u(p.y - q.y, q.x - p.x);
      Parameter k = r/u.dot(u).sqrt();
      st.push_back(o + k*u);
    }
  }
}

void discretize (Edge *e, double eps, vector<PV2> &pts)
{
  if (e->ccw)
    discretize(e->circle->getO(), e->circle->getR(), e->tail->p->getP(), 
	       e->head()->p->getP(), eps, pts);
  else {
    discretize(e->twin, eps, pts);
    reverse(pts.begin(), pts.end());
  }
}

void pe (Edge *e)
{
  vector<PV2> pts;
  discretize(e, 0.01, pts);
  cerr << "(";
  for (vector<PV2>::iterator p = pts.begin();  p != pts.end(); ++p)
    cerr << "(" << p->x.mid() << " " << p->y.mid() << ") ";
  cerr << ")";
}

void pes (Edges &edges)
{
  cerr << "(";
  for (int i = 0; i < edges.size(); ++i)
    pe(edges[i]);
  cerr << ")" << endl;
}

void pl (Edge *e)
{
  Edge *f = e;
  cerr << "(";
  do {
    pe(f);
    f = f->twin->next;
  }
  while (f != e);
  cerr << ") ";
}

void pf (Face *f)
{
  cerr << "(";
  for (Edges::iterator e = f->boundary.begin(); e != f->boundary.end(); ++e)
    pl(*e);
  cerr << ")" << endl;
}


