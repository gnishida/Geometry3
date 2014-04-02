/*
  ACP (Adaptive Controlled Precision) Library
  for robust computational geometry

  Copyright (c) 2013-07-15
  Victor Joseph Milenkovic
  University of Miami
  vjm@miami.edu
  Elisha Sacks
  Purdue University
  eps@cs.purdue.edu

  This file contains code described in

Robust Complete Path Planning in the Plane
Victor Milenkovic, Elisha Sacks, and Steven Trac
Proceedings of the Workshop on the Algorithmic Foundations of Robotics (WAFR)
pages 37-52, 2012

   This code is under development.
   It is free for use for academic and research purposes.
   Permission from the authors is required for commercial use.
*/

#ifndef ACP_H
#define ACP_H

#include <gmp.h>
#include <mpfr.h>
#include <assert.h>
#include <iostream>
#include <fenv.h>
#include <exception>
#include <math.h>
#include <algorithm>
#include <float.h>
#include <qd/qd_real.h>

namespace acp {

double randomNumber (double rmin, double rmax);

class SignException : public std::exception {
 public:
  virtual const char* what() const throw() {
    return "Not enough precision";
  }
};

class PrecisionException : public std::exception {
 public:
  virtual const char* what() const throw() {
    return "Maximum precision exceeded";
  }
};

extern SignException signException;
extern PrecisionException precisionException;

enum RoundMode { RoundUp=1, RoundDown=-1, RoundNearest=0 };

class QValue {
 public:
  void operator= (const QValue& q) { r = q.r; }
  QValue (const QValue& q)
    : r(q.r) {}
  QValue (double ix = 0.0, double iy = 0.0, double iz = 0.0, double iw = 0.0)
    : r(ix, iy, iz, iw) {}
  QValue (const qd_real &ir) : r(ir) {}
  double value () const { return r[0]; }
  QValue plus (const QValue &b, RoundMode round) const {
    qd_round = round;
    return QValue(r + b.r);
  }
  QValue plus (double b, RoundMode round) const {
    qd_round = round;
    return QValue(r + b);
  }
  QValue minus () const { return QValue(-r); }
  QValue minus (const QValue &b, RoundMode round) const {
    return plus(b.minus(), round);
  }
  QValue times (const QValue &b, RoundMode round) const {
    qd_round = round;
    return QValue(r * b.r);
  }
  QValue times (double b, RoundMode round) const {
    qd_round = round;
    return QValue(r * b);
  }
  QValue divide (const QValue &b, RoundMode round) const;
  int sign () const {
    if (r[0] < 0.0 || (r[0] == 0.0 && r[1] < 0.0))
      return -1;
    if (r[0] == 0.0)
      return 0;
    return 1;
  }
  bool operator< (const QValue &b)  const {
    return r < b.r;
  }

  // double &x, &y, &z, &w;
  qd_real r;
};

class MValue {
 public:
  MValue (unsigned int ip) : p(ip) { mpfr_init2(m, p); }
  MValue (double x, unsigned int ip);
  MValue (const MValue &v);
  ~MValue () { mpfr_clear(m); }
  double value () const { return mpfr_get_d(m, GMP_RNDN); }
  MValue plus (const MValue &b, mpfr_rnd_t round) const;
  MValue plus (double b, mpfr_rnd_t round) const;
  MValue minus () const;
  MValue minus (const MValue &b, mpfr_rnd_t round) const;
  MValue times (const MValue &b, mpfr_rnd_t round) const;
  MValue times (double b, mpfr_rnd_t round) const;
  MValue divide (const MValue &b, mpfr_rnd_t round) const;
  int sign () const;
  bool operator< (const MValue &b)  const;
 
  mpfr_t m;
  unsigned int p;
};

class EInt {
 public:
  EInt (const QValue &l, const QValue &u) 
    : refCnt(1), lq(l), uq(u), lm(0), um(0) {}
  EInt (const MValue &l, const MValue &u) 
    : refCnt(1), lm(new MValue(l)), um(new MValue(u)) {}
  ~EInt () { if (lm) { delete lm; delete um; } }
  void incRef () { refCnt++; }
  void decRef () { if (--refCnt == 0) delete this; }

  double intervalWidth () const { 
    return lm ? um->minus(*lm, GMP_RNDN).value()
      : uq.minus(lq, RoundNearest).value();
  }

  double lb () const {
    if (lm)
      return mpfr_get_d(lm->m, GMP_RNDD);
    return lq.r[1] >= 0.0 ? lq.r[0] : nextafter(lq.r[0], - DBL_MAX);
  }

  double ub () const {
    if (lm)
      return mpfr_get_d(um->m, GMP_RNDU);
    return uq.r[1] <= 0.0 ? uq.r[0] : nextafter(uq.r[0], DBL_MAX);
  }

  EInt * plus (const EInt &b) const {
    if (lm)
      return new EInt(lm->plus(*b.lm, GMP_RNDD), um->plus(*b.um, GMP_RNDU));
    return new EInt(lq.plus(b.lq, RoundDown), uq.plus(b.uq, RoundUp));
  }

  EInt * plus (double b) const {
    if (lm)
      return new EInt(lm->plus(b, GMP_RNDD), um->plus(b, GMP_RNDU));
    return new EInt(lq.plus(b, RoundDown), uq.plus(b, RoundUp));
  }

  EInt * minus (const EInt &b) const {
    if (lm)
      return new EInt(lm->minus(*b.um, GMP_RNDD), um->minus(*b.lm, GMP_RNDU));
    return new EInt(lq.minus(b.uq, RoundDown), uq.minus(b.lq, RoundUp));
  }

  EInt * minus () const { 
    if (lm)
      return new EInt(um->minus(), lm->minus());
    return new EInt(uq.minus(), lq.minus()); 
  }

  EInt * times (const EInt &b) const;

  EInt * times (double b) const {
    if (lm)
      return b > 0.0 ? new EInt(lm->times(b, GMP_RNDD), um->times(b, GMP_RNDU))
      : new EInt(um->times(b, GMP_RNDD), lm->times(b, GMP_RNDU));
    return b > 0.0 ? new EInt(lq.times(b, RoundDown), uq.times(b, RoundUp))
      : new EInt(uq.times(b, RoundDown), lq.times(b, RoundUp));
  }

  EInt * divide (const EInt &b) const;

  int sign () const {
    if (lm) {
      if (lm->sign() == 1) return 1;
      if (um->sign() == -1) return -1;
      return 0;
    }
    if (lq.sign() == 1) return 1;
    if (uq.sign() == -1) return -1;
    return 0;
  }

  EInt * mid () const {
    if (lm) {
      MValue m = lm->plus(*um, GMP_RNDN).times(0.5, GMP_RNDN);
      return new EInt(m, m);
    }
    QValue m = lq.plus(uq, RoundNearest).times(0.5, RoundNearest);
    return new EInt(m, m);
  }

  bool subset (const EInt &b) const {
    if (lm)
      return !(*lm < *b.lm) && !(*b.um < *um) && (*b.lm < *lm || *um < *b.um);
    return !(lq < b.lq) && !(b.uq < uq) && (b.lq < lq || uq < b.uq);
  }

  EInt * interval (const EInt &b) const {
    if (lm)
      return new EInt(*lm, *b.um);
    return new EInt(lq, b.uq);
  }

  EInt * intersect (const EInt &b) const {
    if (lm) {
      assert(!(*um < *b.lm && *b.um < *lm));
      MValue *l = *lm < *b.lm ? b.lm : lm;
      MValue *u = *um < *b.um ? um : b.um;
      return new EInt(*l, *u);
    }
    else {
      assert(!(uq < b.lq && b.uq < lq));
      QValue l = lq < b.lq ? b.lq : lq;
      QValue u = uq < b.uq ? uq : b.uq;
      return new EInt(l, u);
    }
  }

  unsigned int refCnt;
  QValue lq, uq;
  MValue *lm, *um;
};

class Object;
class AnglePoly;
class PFun;
class PPoly1;
class PPoly2;

// The number class:  an interval of double with multiple precision backup.
// Treat it like a double.
class Parameter {
  friend class MValue;
  friend class Object;
  friend class AnglePoly;
  friend class PPoly1;
  friend class PPoly2;

 public:
  // Perturbation magnitude.
  static double delta; // default is 2^{-26}

  // Maximum allowed precision.  Default is 848 bits.
  static unsigned int maxPrecision;

  // Call Parameter::enable() before using ACP
  static void enable () { 
    if (!enabled) { enabled = true; fesetround(FE_UPWARD); }
  }

  // Call Parameter::disable() before non-ACP calculations.
  static void disable () { 
    if (enabled) { enabled = false; fesetround(FE_TONEAREST); }
  }

  Parameter () : l(0.0) { u.r = 0.0; }

  // Should be zero only if created by zero-argument constructor.
  bool uninitialized () const { return l == 0.0 && u.r == 0.0; }

  // Construct from double.  Will be perturbed.
  Parameter (double x) {
    l = u.r = x + delta*(1.0 + fabs(x))*randomNumber(-1.0, 1.0);
  }

  static Parameter constant (double x) { return Parameter(x, x); }

  Parameter (const Parameter &p) : l(p.l) {
    if (l != sentinel)
      u.r = p.u.r;
    else {
      u.e = p.u.e;
      u.e->incRef();
    }
  }
  
  ~Parameter () { 
    if (l == sentinel)
      u.e->decRef();
  }
  
  Parameter & operator= (const Parameter &p) {
    if (p.l == sentinel)
      p.u.e->incRef();
    if (l == sentinel)
      u.e->decRef();
    l = p.l;
    u = p.u;
    return *this;
  }
  
  // Determine the sign of a parameter: -1 or 1.
  // If fail==true, will return 0 on sign failure.
  // Otherwise throws SignException.
  int sign (bool fail = true) const {
    if (l != sentinel) {
      if (l > 0.0)
	return 1;
      if (u.r < 0.0)
	return -1;
      if (fail)
	throw signException;
      return 0;
    }
    int s = u.e->sign();
    if (s)
      return s;
    if (fail) {
      highPrecision *= 2;
      if (highPrecision <= maxPrecision)
        throw signException;
      else
        throw precisionException;
    }
    return 0;
  }
  
  // Approximate value of parameter for display purposes.
  double mid () const { return 0.5*(lb() + ub()); }

  // double precision lower bound of interval
  double lb () const { return l != sentinel ? l : u.e->lb(); }

  // double precision upper bound of interval
  double ub () const { return l != sentinel ? u.r : u.e->ub(); }

  // Parameter has unary - and binary +,-,*,/, <, >
  // If one argument of binary operation is a double instead of a
  // Parameter, it is treated as a constant, not perturbed.
  // Hence 2 * p does not perturb the 2.
  Parameter operator+ (const Parameter &b) const {
    if (l != sentinel && b.l != sentinel)
      return Parameter(- (- l - b.l), u.r + b.u.r);
    assert(l == sentinel && b.l == sentinel);
    return Parameter(u.e->plus(*b.u.e));
  }

  Parameter operator+ (double b) const {
    if (l != sentinel)
      return Parameter(- (- l - b), u.r + b);
    return Parameter(u.e->plus(b));
  }
  
  Parameter operator- (const Parameter &b) const {
    if (l != sentinel && b.l != sentinel)
      return Parameter(- (b.u.r - l), u.r - b.l);
    assert(l == sentinel && b.l == sentinel);
    return Parameter(u.e->minus(*b.u.e));
  }
  
  Parameter operator- (double b) const { return *this + (- b); }

  Parameter operator- () const {
    if (l != sentinel)
      return Parameter(- u.r, - l);
    return Parameter(u.e->minus());
  }
  
  Parameter operator* (const Parameter &b) const {
    if (l != sentinel && b.l != sentinel) {
      Parameter s = u.r < 0.0 ? - *this : *this, t = u.r < 0.0 ? - b : b;
      if (s.l > 0.0) {
	double k = t.l > 0.0 ? - s.l : - s.u.r;
	return Parameter(- (k*t.l), t.u.r > 0.0 ? s.u.r*t.u.r : s.l*t.u.r);
      }
      if (t.l > 0.0) {
	double k = - s.l;
	return Parameter(- (k*t.u.r), s.u.r*t.u.r);
      }
      if (t.u.r < 0.0) {
	double k = - s.u.r;
	return Parameter(- (k*t.l), s.l*t.l);
      }
      double k1 = - s.l, k2 = - s.u.r, cl1 = k1*t.u.r, cl2 = k2*t.l, 
	cu1 = s.l*t.l, cu2 = s.u.r*t.u.r,
	cl = cl1 < cl2 ? - cl2 : - cl1, cu = cu1 < cu2 ? cu2 : cu1;
      return Parameter(cl, cu);
    }
    assert(l == sentinel && b.l == sentinel);
    return Parameter(u.e->times(*b.u.e));
  }
  
  Parameter operator* (double b) const {
    if (l != sentinel) {
      double k = - b;
      return b > 0.0 ? Parameter(- (k*l), b*u.r) : Parameter(- (k*u.r), b*l);
    }
    return Parameter(u.e->times(b));
  }

  Parameter operator/ (const Parameter &b) const {
    int bs = b.sign();
    if (l != sentinel && b.l != sentinel) {
      if (bs == 1) {
	if (l >= 0.0)
	  return Parameter(- ((- l)/b.u.r), u.r/b.l);
	if (u.r <= 0.0)
	  return Parameter(- ((- l)/b.l), u.r/b.u.r);
	return Parameter(- ((- l)/b.u.r), u.r/b.u.r);
      }
      if (l >= 0.0)
	return Parameter(- ((- u.r)/b.u.r), l/b.l);
      if (u.r <= 0.0)
	return Parameter(- ((- u.r)/b.l), l/b.u.r);
      return Parameter(- ((- u.r)/b.l), l/b.l);
    }
    assert(l == sentinel && b.l == sentinel);
    return Parameter(u.e->divide(*b.u.e));
  }

  Parameter operator/ (double b) const { return *this*(1.0/b); }
    
  bool operator< (const Parameter &b) const { return (b - *this).sign() == 1; }
  bool operator< (double b) const { return (*this - b).sign() == -1; }
  bool operator> (const Parameter &b) const { return (b - *this).sign() == -1; }
  bool operator> (double b) const { return (*this - b).sign() == 1; }

  Parameter abs () const { 
    return sign() == 1 ? *this : -*this; 
  }

  Parameter sqrt () const;

 private:
  static const double sentinel;
  static unsigned int highPrecision;
  static bool enabled;
  static bool quadDouble;
  static unsigned int controlWord;

  Parameter (double il, double iu) : l(il) { u.r = iu; }    

  Parameter (EInt *e) : l(sentinel) { u.e = e; }

  Parameter lbP () const {
    if (l != sentinel)
      return Parameter(l, l);
    if (u.e->lm == 0)
      return Parameter(new EInt(u.e->lq, u.e->lq));
    else
      return Parameter(new EInt(*u.e->lm, *u.e->lm));
  }

  Parameter ubP () const {
    if (l != sentinel)
      return Parameter(u.r, u.r);
    if (u.e->lm == 0)
      return Parameter(new EInt(u.e->uq, u.e->uq));
    else
      return Parameter(new EInt(*u.e->um, *u.e->um));
  }

  unsigned int precision () const {
    return l != sentinel ? 53u : (!u.e->lm ? 212u : u.e->lm->p);
  }

  bool increased () const { return precision() == highPrecision; }
  bool decreased () const { return precision() == 53u; }

 public:
  void increasePrecision () {
    if (l == sentinel && 
	(highPrecision == 212u || (u.e->lm && u.e->lm->p == highPrecision)))
      return;
    double il = lb(), iu = ub();
    disable();
    if (l == sentinel)
      u.e->decRef();
    l = sentinel;
    if (highPrecision == 212u) {
      if (!quadDouble) {
        //fpu_fix_start(&controlWord);
        quadDouble = true;
      }
      u.e = new EInt(QValue(il), QValue(iu));
    }
    else {
      if (quadDouble) {
        //fpu_fix_end(&controlWord);
        quadDouble = false;
      }
      u.e = new EInt(MValue(il, highPrecision), MValue(iu, highPrecision));
    }
  }

  void decreasePrecision () {
    if (l == sentinel) {
      Parameter::highPrecision = 212u;
      enable();
      if (quadDouble) {
        //fpu_fix_end(&controlWord);
        quadDouble = false;
      }
      double dl = lb(), du = ub();
      u.e->decRef();
      l = dl;
      u.r = du;
    }
  }

  Parameter midP () const {
    if (l != sentinel)
      return constant(0.5*(l + u.r));
    return Parameter(u.e->mid());
  }

  double intervalWidth () const {
    return l != sentinel ? u.r - l : u.e->intervalWidth();
  }
  
  bool subset (const Parameter &b) const {
    if (l != sentinel && b.l != sentinel)
      return !(l < b.l) && !(b.u.r < u.r) && (b.l < l || u.r < b.u.r);
    assert (l == sentinel && b.l == sentinel);
    return u.e->subset(*b.u.e);
  }

  Parameter interval (const Parameter &b) const {
    if (l != sentinel && b.l != sentinel)
      return Parameter(l, b.u.r);
    assert (l == sentinel && b.l == sentinel);
    return Parameter(u.e->interval(*b.u.e));
  }

  Parameter intersect (const Parameter &b) const {
    if (l != sentinel && b.l != sentinel) {
      assert(!(u.r < b.l && b.u.r < l));
      double il = l < b.l ? b.l : l;
      double iu = u.r < b.u.r ? u.r : b.u.r;
      return Parameter(il, iu);
    }
    assert (l == sentinel && b.l == sentinel);
    return Parameter(u.e->intersect(*b.u.e));
  }

  double l;
  union {
    double r;
    EInt *e;
  } u;
};

inline Parameter operator+ (double a, const Parameter &b)
{
  return b + a;
}

inline Parameter operator- (double a, const Parameter &b)
{
  return (- b) + a;
}

inline Parameter operator* (double a, const Parameter &b)
{
  return b*a;
}

inline Parameter operator/ (double a, const Parameter &b)
{
  return Parameter::constant(a) / b;
}

inline bool operator< (double a, const Parameter &b)
{
  return (b - a).sign() == 1;
}

inline bool operator> (double a, const Parameter &b)
{
  return (b - a).sign() == -1;
}

}
#endif
