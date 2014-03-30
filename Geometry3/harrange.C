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

bool Edge::clockwise (Edge *e)
{
  bool inc = increasingY(), einc = e->increasingY();
  return inc != einc ? inc : LeftTurn(head()->p, tail->p, e->head()->p) == 1;
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

Edge * Arrangement::addEdge (Vertex *tail, Vertex *head, bool flag)
{
  Edge *e = addHalfEdge(tail, 0, 0, true, flag),
    *et = addHalfEdge(head, e, 0, false, flag);
  e->twin = et;
  tail->addEdge(e);
  head->addEdge(et);
  return e;
}

Edge * Arrangement::addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool in, bool flag)
{
  Edge *e = new Edge(tail, twin, next, in, flag);
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

void Arrangement::formFaces ()
{
	for (Faces::iterator f = faces.begin(); f != faces.end(); ++f)
		delete *f;
	faces.clear();

	Edges inner;
	faces.push_back(new Face);
	for (Edges::iterator e = edges.begin(); e != edges.end(); ++e) {
		if (!(*e)->flag) {
			Edge *l = (*e)->formLoop();

			Face *f = new Face;
			faces.push_back(f);
		}
	}
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

