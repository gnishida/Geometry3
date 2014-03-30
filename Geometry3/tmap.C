#include "tmap.h"

Tnode * Slab::update (Tnode *bnode, Tnode *tnode) const
{
  Edge *e = bnode->l->top;
  Tnode *node = new Tnode(e, bnode, tnode);
  linkb(bnode->l, br);
  linkt(tnode->l, tr);
  if (e->head() != rightp && XOrder(e->head()->p, rightp->p) == 1) {
    Slab *slast = new Slab(bottom, top, e->head(), rightp);
    linkb(bnode->l, slast);
    linkt(tnode->l, slast);
    linkb(slast, br);
    linkt(slast, tr);
    Tnode *nlast = new Tnode(e->head(), node, new Tnode(slast));
    node = nlast;
  }
  if (leftp == e->tail) {
    linkb(bl, bnode->l);
    linkt(tl, tnode->l);
  }
  else if (XOrder(leftp->p, e->tail->p) == 1) {
    Slab *sfirst = new Slab(bnode->l->bottom, tnode->l->top, leftp, e->tail);
    linkb(bl, sfirst);
    linkt(tl, sfirst);
    linkb(sfirst, bnode->l);
    linkt(sfirst, tnode->l);
    Tnode *nfirst = new Tnode(e->tail, new Tnode(sfirst), node);
    node = nfirst;
  }
  return node;
}

void linkb (Slab *l, Slab *r)
{
  if (l) l->br = r;
  if (r) r->bl = l;
}

void linkt (Slab *l, Slab *r)
{
  if (l) l->tr = r;
  if (r) r->tl = l;
}

Tnode::~Tnode ()
{ 
  if (leaf) 
    delete l;
  else {
    if (i->left) delete i->left;
    if (i->right) delete i->right;
    delete i;
  }
  for (Tnodes::iterator p = parents.begin(); p != parents.end(); ++p)
    if ((*p)->i->left == this)
      (*p)->i->left = 0;
    else
      (*p)->i->right = 0;
}

bool TnodeInternal::above (Edge *f) const
{
  return e->tail == f->tail ?
    LeftTurn(e->head()->p, e->tail->p, f->head()->p) == 1
  : LeftTurn(f->tail->p, e->head()->p, e->tail->p) == 1;
}

bool TnodeInternal::above (Point *p) const
{
  return LeftTurn(p, e->head()->p, e->tail->p) == 1;
}

Tmap::Tmap (Face *f0, double *bbox) : f0(f0)
{
  bl = new Vertex(new InputPoint(bbox[0], bbox[2]));
  tr = new Vertex(new InputPoint(bbox[1], bbox[3]));
  Slab *s = new Slab(0, 0, bl, tr);
  root = new Tnode(s);
}

void Tmap::insert (Edge *e)
{
  Slab *slab = find(e)->l, *nslab;
  Slab *bslab = new Slab(slab->bottom, e, e->tail, e->head());
  Slab *tslab = new Slab(e, slab->top, e->tail, e->head());
  Tnode *bnode = new Tnode(bslab), *tnode = new Tnode(tslab);
  while (true) {
    bool tflag, done = e->head() == slab->rightp ||
      XOrder(e->head()->p, slab->rightp->p) == 1;
    if (!done) {
      tflag = LeftTurn(slab->rightp->p, e->tail->p, e->head()->p) == -1;
      nslab = tflag ? slab->tr : slab->br;
    }
    update(slab, bnode, tnode);
    if (done)
      break;
    slab = nslab;
    if (tflag) {
      bslab->rightp = slab->leftp;
      Slab *nbslab = new Slab(slab->bottom, e, slab->leftp, e->head());
      linkt(bslab, nbslab);
      linkb(slab->bl, nbslab);
      bslab = nbslab;
      bnode = new Tnode(bslab);
    }
    else {
      tslab->rightp = slab->leftp;
      Slab *ntslab = new Slab(e, slab->top, slab->leftp, e->head());
      linkb(tslab, ntslab);
      linkt(slab->tl, ntslab);
      tslab = ntslab;
      tnode = new Tnode(tslab);
    }
  }
}

void Tmap::update (Slab *slab, Tnode *bnode, Tnode *tnode)
{
  Tnode *onode = slab->node, *nnode = slab->update(bnode, tnode);
  if (onode == root) {
    delete root;
    root = nnode;
    return;
  }
  for (Tnodes::iterator p = onode->parents.begin();
       p != onode->parents.end(); ++p) {
    if ((*p)->i->left == onode)
      (*p)->i->left = nnode;
    else
      (*p)->i->right = nnode;
  }
  nnode->parents = onode->parents;
  onode->parents.clear();
  delete onode;
}

const Tnode * Tmap::find (Edge *e) const
{
  Vertex *v = e->tail;
  Tnode *node = root;
  while (!node->leaf) {
    bool lflag = node->i->xnode ?
      v != node->i->v && XOrder(v->p, node->i->v->p) == 1 :
      node->i->above(e);
    if (lflag)
      node = node->i->left;
    else
      node = node->i->right;
  }
  return node;
}

Face * Tmap::find (Point *p) const
{
  if (XOrder(p, bl->p) == 1 || XOrder(tr->p, p) == 1 ||
      YOrder(p, bl->p) == 1 || YOrder(tr->p, p) == 1)
    return f0;
  Tnode *node = root;
  while (!node->leaf) {
    bool lflag = node->i->xnode ?
      XOrder(p, node->i->v->p) == 1 : node->i->above(p);
    if (lflag)
      node = node->i->left;
    else
      node = node->i->right;
  }
  if (node->l->bottom)
    return node->l->bottom->face;
  if (node->l->top)
    return node->l->top->twin->face;
  return f0;
}

Tmap * formTmap (Arrangement *arr, double *bbox)
{
  Tmap *tmap = new Tmap(arr->faces[0], bbox);
  int n = arr->edges.size(), *p = new int [n];
  randomPermutation(n, p);
  for (int i = 0; i < n; ++i) {
    Edge *e = arr->edges[p[i]];
    if (e->increasingX())
      tmap->insert(e);
  }
  delete [] p;
  return tmap;
}
