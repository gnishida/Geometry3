#include "circle.h"

int quadrant (Circle *c, Point *p)
{
  CircleCenter o(c);
  bool xpos = XOrder(&o, p) == 1, ypos = YOrder(&o, p) == 1;
  if (xpos)
    return ypos ? 1 : 4;
  return ypos ? 2 : 3;
}

int PointInCircle::sign ()
{
  PV2 u = p->getP() - c->getO();
  return (c->getR()*c->getR() - u.dot(u)).sign();
}

int CircleOrder::sign ()
{
  return (a->getP() - c->getO()).cross(b->getP() - c->getO()).sign();
}

int IntersectingCircles::sign ()
{
  PV2 u = d->getO() - c->getO();
  Parameter dd = u.dot(u), r = c->getR(), s = d->getR();
  return dd < (r - s)*(r - s) || dd > (r + s)*(r + s) ? -1 : 1;
}

int IntersectionSign::sign ()
{
  return (a->getP() - c->getO()).cross(a->getP() - d->getO()).sign();
}

PV2 circumCenter (const PV2 &a, const PV2 &b, const PV2 &c)
{
  Parameter xy[2];
  linsolve2(a.y - b.y, c.y - a.y, b.x - a.x, a.x - c.x, 
	    0.5*(c.x - b.x), 0.5*(c.y - b.y), xy);
  return PV2(0.5*(a.x + b.x) + xy[0]*(a.y - b.y),
	     0.5*(a.y + b.y) + xy[0]*(b.x - a.x));
}

void linsolve2 (Parameter a, Parameter b, Parameter c, Parameter d, 
		Parameter e, Parameter f, Parameter *xy)
{
  Parameter den = a*d - b*c;
  xy[0] = (e*d - b*f)/den;
  xy[1] = (a*f - c*e)/den;
}

void circleIntersection (const PV2 &a, const Parameter &r, const PV2 &b, 
			 const Parameter &s, PV2 &p1, PV2 &p2)
{
  PV2 ab = b - a;
  Parameter dd = ab.dot(ab), d = dd.sqrt(),
    ct = (r*r + dd - s*s)/(2.0*r*d),
    st = (1.0 - ct*ct).sqrt();
  PV2 u = (r/d)*ab;
  p1.x = a.x + ct*u.x - st*u.y;
  p1.y = a.y + st*u.x + ct*u.y;
  p2.x = a.x + ct*u.x + st*u.y;
  p2.y = a.y - st*u.x + ct*u.y;
}
