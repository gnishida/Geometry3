#ifndef CIRCLE
#define CIRCLE

#include "point.h"

class Circle : public Object {
 private:
  Parameters getParameters () { return Parameters(r, o); }
 protected:
  PV2 o;
  Parameter r;
 public:
  PV2 getO () { return o; }
  Parameter getR () { return r; }
};

typedef vector<Circle *> Circles;

int quadrant (Circle *c, Point *p);

Predicate2(PointInCircle, Point *, p, Circle *, c);

Predicate3(CircleOrder, Circle *, c, Point *, a, Point *, b);

Predicate2(IntersectingCircles, Circle *, c, Circle *, d);

Predicate3(IntersectionSign, Circle *, c, Circle *, d, Point *, a);

class Circle2pts : public Circle {
 private:
  Objects getObjects () { return Objects(a, b); }
  void calculate () {
    o = 0.5*(a->getP() + b->getP());
    PV2 u = o - a->getP();
    r = u.dot(u).sqrt();
  }
  Point *a, *b;
 public:
  Circle2pts (Point *a, Point *b) : a(a), b(b) { calculate(); }
};

PV2 circumCenter (const PV2 &a, const PV2 &b, const PV2 &c);

void linsolve2 (Parameter a, Parameter b, Parameter c, Parameter d, 
		Parameter e, Parameter f, Parameter *xy);

class Circle3pts : public Circle {
 private:
  Objects getObjects () { return Objects(a, b, c); }
  void calculate () {
    o = circumCenter(a->getP(), b->getP(), c->getP());
    PV2 u = o - a->getP();
    r = u.dot(u).sqrt();
  }
  Point *a, *b, *c;
 public:
  Circle3pts (Point *a, Point *b, Point *c) : a(a), b(b), c(c)
  { calculate(); }
};

class CircleCenter : public Point {
 private:
  Objects getObjects () { return Objects(c); }
  void calculate () { p = c->getO(); }
 protected:
  Circle *c;
 public:
  CircleCenter (Circle *c) : c(c) { calculate(); }
  CircleCenter * copy () const { return new CircleCenter(c); }
  Circle * getC () { return c; }
};

class CircleTangent : public Point {
 private:
  Objects getObjects () { return Objects(c, a); }
  void calculate () { 
    PV2 n = (a->getP() - c->getO())/c->getR();
    p.x = ccw ? - n.y : n.y;
    p.y = ccw ? n.x : - n.x;
  }
 protected:
  Circle *c;
  Point *a;
  bool ccw;
 public:
  CircleTangent (Circle *c, Point *a, bool ccw) 
    : c(c), a(a), ccw(ccw) { calculate(); }
  CircleTangent * copy () const { return new CircleTangent(c, a, ccw); }
};

class CircleTurningPoint : public Point {
 private:
  Objects getObjects () { return Objects(c); }
  void calculate () { 
    p = c->getO();
    switch (t) {
    case 1: p.x = p.x + c->getR(); break;
    case 2: p.y = p.y + c->getR(); break;
    case 3: p.x = p.x - c->getR(); break;
    case 4: p.y = p.y - c->getR(); break;
    }
  }
 protected:
  Circle *c;
  int t;
 public:
  CircleTurningPoint (Circle *c, int t) : c(c), t(t) { calculate(); }
  CircleTurningPoint * copy () const { return new CircleTurningPoint(c, t); }
  int getT () const { return t; }
};

void circleIntersection (const PV2 &a, const Parameter &r, const PV2 &b, 
			 const Parameter &s, PV2 &p1, PV2 &p2);

class CircleIntersection : public Point {
 private:
  Objects getObjects () { return Objects(c, d); }
  void calculate () {
    PV2 q1, q2;
    circleIntersection(c->getO(), c->getR(), d->getO(), d->getR(), q1, q2);
    p = first ? q1 : q2;
  }
 protected:
  Circle *c, *d;
  bool first;
 public:
  CircleIntersection (Circle *c, Circle *d, bool first) 
    : c(c), d(d), first(first) { calculate(); }
  CircleIntersection * copy () const { 
    return new CircleIntersection(c, d, first); 
  }
  Circle * getC () { return c; }
  Circle * getD () { return d; }
};

#endif
