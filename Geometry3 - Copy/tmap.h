#ifndef TMAP
#define TMAP

#include "arrange.h"
#include "permute.h"

class Tnode;

class Slab {
 public:
  Slab (Edge *bottom, Edge *top, Vertex *leftp, Vertex *rightp,
	Slab *bl = 0, Slab *br = 0, Slab *tl = 0, Slab *tr = 0)
    : bottom(bottom), top(top), leftp(leftp), rightp(rightp), 
    bl(bl), br(br), tl(tl), tr(tr), node(0) {}

  Tnode * update (Tnode *bnode, Tnode *tnode) const;

  Edge *bottom, *top;
  Vertex *leftp, *rightp;
  Slab *bl, *br, *tl, *tr;
  Tnode *node;
};

void linkb (Slab *l, Slab *r);

void linkt (Slab *l, Slab *r);

class TnodeInternal {
 public:
  TnodeInternal () : xnode(true), left(0), right(0) {}
  TnodeInternal (Vertex *v, Tnode *left, Tnode *right)
    : xnode(true), v(v), left(left), right(right) {}
  TnodeInternal (Edge *e, Tnode *left, Tnode *right)
    : xnode(false), e(e), left(left), right(right) {}
  bool above (Edge *f) const;
  bool above (Point *p) const;

  bool xnode;
  union {
    Vertex *v;
    Edge *e;
  };
  Tnode *left, *right;
};

class Tnode {
 public:
  Tnode (Slab *slab) : leaf(true), l(slab) { slab->node = this; }

  Tnode (Vertex *v, Tnode *left, Tnode *right)
    : leaf(false), i(new TnodeInternal(v, left, right))      
    { left->parents.push_back(this); right->parents.push_back(this); }
  
  Tnode (Edge *e, Tnode *left, Tnode *right)
    : leaf(false), i(new TnodeInternal(e, left, right))
    { left->parents.push_back(this); right->parents.push_back(this); }

  ~Tnode ();

  bool leaf;
  union {
    Slab *l;
    TnodeInternal *i;
  };
  vector<Tnode *> parents;
};

typedef vector<Tnode *> Tnodes;

class Tmap {
 public:
  Tmap (Face *f0, double *bbox);
  ~Tmap () { delete root; delete bl; delete tr; }
  void insert (Edge *e);
  void update (Slab *slab, Tnode *bnode, Tnode *tnode);
  const Tnode * find (Edge *e) const;
  Face * find (Point *p) const;

  Vertex *bl, *tr;
  Tnode *root;
  Face *f0;
};

Tmap * formTmap (Arrangement *arr, double *bbox);

#endif
