#include "triangulate.h"

void triangulate (Arrangement *arr, double *bbox)
{
  addBBox(arr, bbox);
  makeMonotone(arr);
  triangulateMonotone(arr);
}

void addBBox (Arrangement *arr, double *bbox)
{
  Vertex *a = arr->addVertex(new InputPoint(bbox[0], bbox[2])),
    *b = arr->addVertex(new InputPoint(bbox[1], bbox[2])),
    *c = arr->addVertex(new InputPoint(bbox[1], bbox[3])),
    *d = arr->addVertex(new InputPoint(bbox[0], bbox[3]));
  arr->addEdge(a, b)->twin->flag = true;
  arr->addEdge(b, c)->twin->flag = true;
  arr->addEdge(c, d)->twin->flag = true;
  arr->addEdge(d, a)->twin->flag = true;
}

void makeMonotone (Arrangement *arr)
{
  sort(arr->vertices.begin(), arr->vertices.end(), VertexYOrder);
  Sweep sweep;
  for (Vertices::iterator v = arr->vertices.begin(); 
       v != arr->vertices.end(); ++v)
    makeMonotoneVertex(arr, sweep, *v);
}

void makeMonotoneVertex (Arrangement *arr, Sweep &sweep, Vertex *v)
{
  Vertices d;
  Edge *e = v->edge, *f = 0;
  while (e->increasingY()) {
    if (!f)
      f = e->pred();
    if (e->helper->flag)
      d.push_back(e->helper);
    sweep.remove(e);
    e = e->next;
    if (e == v->edge)
      break;
  }
  v->flag = true;
  while (!e->increasingY()) {
    v->flag = false;
    e->twin->helper = v;
    sweep.insert(e->twin);
    if (!f)
      f = e->twin->pred();
    e = e->next;
    if (e == v->edge)
      break;
  }
  if (f) {
    if (f->helper->flag || !v->edge->increasingY())
      d.push_back(f->helper);
    f->helper = v;
  }
  for (Vertices::iterator w = d.begin(); w != d.end(); ++w)
    arr->addEdge(v, *w);
}

void triangulateMonotone (Arrangement *arr)
{
  for (Vertices::iterator v = arr->vertices.begin(); 
       v != arr->vertices.end(); ++v) {
    Edge *e0 = (*v)->edge, *e = e0;
    do {
      if (!(e->flag || e->increasingY())) {
	Vertices u;
	monotoneLoop(e, u);
	triangulateLoop(u, arr);
      }
      e = e->next;
    }
    while (e != e0);
  }
}

void monotoneLoop (Edge *e, Vertices &u)
{
  bool rflag = false;
  while (!e->flag) {
    rflag = rflag || e->increasingY();
    e->tail->flag = rflag;
    u.push_back(e->tail);
    e->flag = true;
    e = e->twin->next;
  }
  sort(u.begin(), u.end(), VertexYOrder);
}

void triangulateLoop (const Vertices &u, Arrangement *arr)
{
  Vertices stack;
  stack.push_back(u[0]);
  stack.push_back(u[1]);
  int n = u.size();
  for (int i = 2; i + 1 < n; ++i)
    if (u[i]->flag == (*(stack.end()-1))->flag)
      triangulateSame(u[i], stack, arr);
    else
      triangulateOther(u[i], stack, arr);
  for (int i = 1; i + 1 < stack.size(); ++i)
    arr->addEdge(u[n-1], stack[i], true, true);
}

void triangulateSame (Vertex *v, Vertices &stack, Arrangement *arr)
{
  int n = stack.size() - 1;
  while (n > 0) {
    bool aflag = LeftTurn(stack[n-1]->p, stack[n]->p, v->p) == 1;
    if (v->flag ? aflag : !aflag)
      break;
    arr->addEdge(v, stack[n-1], true, true);
    stack.pop_back();
    --n;
  }
  stack.push_back(v);
}

void triangulateOther (Vertex *v, Vertices &stack, Arrangement *arr)
{
  int n = stack.size() - 1;
  Vertex *q = stack[n];
  bool flag = q->flag;
  while (n > 0) {
    arr->addEdge(v, stack[n], true, true);
    --n;
  }
  stack.clear();
  stack.push_back(q);
  stack.push_back(v);
}

bool VertexYOrder (Vertex *a, Vertex *b)
{
  return a != b && YOrder(a->p, b->p) == -1;
}
