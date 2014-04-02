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

#include "acp.h"
using namespace acp;

namespace acp {

int qd_round;

double randomNumber (double rmin, double rmax)
{
  return rmin + (rmax - rmin)*random()/double(RAND_MAX);
}

QValue QValue::divide (const QValue &b, RoundMode round) const
{
  double q0, q1, q2, q3, q4;
  q0 = r[0]/b.r[0];
  QValue q = minus(b.times(q0, RoundNearest), RoundNearest);
  q1 = q.r[0]/b.r[0];
  q = q.minus(b.times(q1, RoundNearest), RoundNearest);
  q2 = q.r[0]/b.r[0];
  q = q.minus(b.times(q2, RoundNearest), RoundNearest);
  q3 = q.r[0]/b.r[0];
  q = q.minus(b.times(q3, RoundNearest), RoundNearest);
  q4 = q.r[0]/b.r[0];
  qd_round = round;
  qd::renorm(q0, q1, q2, q3, q4);
  return QValue(q0, q1, q2, q3);
}

EInt * EInt::times (const EInt &b) const
{
  if (lm) {
    bool pflag = um->sign() == -1;
    MValue sl = pflag ? um->minus() : *lm, su = pflag ? lm->minus() : *um,
    tl = pflag ? b.um->minus() : *b.lm, tu = pflag ? b.lm->minus() : *b.um;
    if (sl.sign() == 1) {
      MValue &l1 = tl.sign() == 1 ? sl : su, &u1 = tu.sign() == 1 ? su : sl;
      return new EInt(l1.times(tl, GMP_RNDD), u1.times(tu, GMP_RNDU));
    }
    if (tl.sign() == 1)
      return new EInt(sl.times(tu, GMP_RNDD), su.times(tu, GMP_RNDU));
    if (tu.sign() == -1)
      return new EInt(su.times(tl, GMP_RNDD), sl.times(tl, GMP_RNDU));
    MValue cl1 = sl.times(tu, GMP_RNDD), cl2 = su.times(tl, GMP_RNDD),
      cu1 = sl.times(tl, GMP_RNDU), cu2 = su.times(tu, GMP_RNDU);
    return new EInt(cl1 < cl2 ? cl1 : cl2, cu1 < cu2 ? cu2 : cu1);
  }
  bool pflag = uq.sign() == -1;
  QValue sl = pflag ? uq.minus() : lq, su = pflag ? lq.minus() : uq,
    tl = pflag ? b.uq.minus() : b.lq, tu = pflag ? b.lq.minus() : b.uq;
  if (sl.sign() == 1) {
    QValue &l1 = tl.sign() == 1 ? sl : su, &u1 = tu.sign() == 1 ? su : sl;
    return new EInt(l1.times(tl, RoundDown), u1.times(tu, RoundUp));
  }
  if (tl.sign() == 1)
    return new EInt(sl.times(tu, RoundDown), su.times(tu, RoundUp));
  if (tu.sign() == -1)
    return new EInt(su.times(tl, RoundDown), sl.times(tl, RoundUp));
  QValue cl1 = sl.times(tu, RoundDown), cl2 = su.times(tl, RoundDown),
    cu1 = sl.times(tl, RoundUp), cu2 = su.times(tu, RoundUp);
  return new EInt(cl1 < cl2 ? cl1 : cl2, cu1 < cu2 ? cu2 : cu1);
}

EInt * EInt::divide (const EInt &b) const
{
  int as = sign(), bs = b.sign();
  if (lm) {
    if (bs == 1)
      switch (as) {
      case 1:
	return new EInt(lm->divide(*b.um, GMP_RNDD), um->divide(*b.lm, GMP_RNDU));
      case 0:
	return new EInt(lm->divide(*b.um, GMP_RNDD), um->divide(*b.um, GMP_RNDU));
      case -1:
	return new EInt(lm->divide(*b.lm, GMP_RNDD), um->divide(*b.um, GMP_RNDU));
      }
    switch (as) {
    case 1:
      return new EInt(um->divide(*b.um, GMP_RNDD), lm->divide(*b.lm, GMP_RNDU));
    case 0:
      return new EInt(um->divide(*b.lm, GMP_RNDD), lm->divide(*b.lm, GMP_RNDU));
    case -1:
      return new EInt(um->divide(*b.lm, GMP_RNDD), lm->divide(*b.um, GMP_RNDU));
    }
  }
  if (bs == 1)
    switch (as) {
    case 1:
      return new EInt(lq.divide(b.uq, RoundDown), uq.divide(b.lq, RoundUp));
    case 0:
      return new EInt(lq.divide(b.uq, RoundDown), uq.divide(b.uq, RoundUp));
    case -1:
      return new EInt(lq.divide(b.lq, RoundDown), uq.divide(b.uq, RoundUp));
    }
  switch (as) {
  case 1:
    return new EInt(uq.divide(b.uq, RoundDown), lq.divide(b.lq, RoundUp));
  case 0:
    return new EInt(uq.divide(b.lq, RoundDown), lq.divide(b.lq, RoundUp));
  case -1:
    return new EInt(uq.divide(b.lq, RoundDown), lq.divide(b.uq, RoundUp));
  }
  return 0;
}

MValue::MValue (double x, unsigned int ip) : p(ip) 
{ 
  mpfr_init2(m, p);
  mpfr_set_d(m, x, GMP_RNDN);
}

MValue::MValue (const MValue &v) : p(v.p) 
{ 
  mpfr_init2(m, v.p); 
  mpfr_set(m, v.m, GMP_RNDN); 
}

MValue MValue::plus (const MValue &b, mpfr_rnd_t round) const
{
  MValue res(p);
  mpfr_add(res.m, m, b.m, round);
  return res;
}

MValue MValue::plus (double b, mpfr_rnd_t round) const
{
  MValue res(p);
  mpfr_add_d(res.m, m, b, round);
  return res;
}

MValue MValue::minus () const
{
  MValue res(p);
  mpfr_neg(res.m, m, GMP_RNDN);
  return res;
}

MValue MValue::minus (const MValue &b, mpfr_rnd_t round) const
{
  MValue res(p);
  mpfr_sub(res.m, m, b.m, round);
  return res;
}

MValue MValue::times (const MValue &b, mpfr_rnd_t round) const
{
  MValue res(p);
  mpfr_mul(res.m, m, b.m, round);
  return res;
}

MValue MValue::times (double b, mpfr_rnd_t round) const
{
  MValue res(p);
  mpfr_mul_d(res.m, m, b, round);
  return res;
}

MValue MValue::divide (const MValue &b, mpfr_rnd_t round) const
{
  MValue res(p);
  mpfr_div(res.m, m, b.m, round);
  return res;
}

int MValue::sign () const
{
  return mpfr_sgn(m);
}

bool MValue::operator< (const MValue &b)  const
{
  return mpfr_less_p(m, b.m);
}

double Parameter::delta = std::pow(2.0, -27);
const double Parameter::sentinel = 1e20;
unsigned int Parameter::highPrecision = 212u;
unsigned int Parameter::maxPrecision = 848u;
bool Parameter::enabled = false;
bool Parameter::quadDouble = false;
unsigned int Parameter::controlWord = 0;
SignException signException;
PrecisionException precisionException;

Parameter Parameter::sqrt () const {
  assert(sign() > 0);
  double myUB = ub();
  double myLB = lb();
  double s = ::sqrt((myUB + myLB) / 2);
  if (l == sentinel)
    enable();
  double s1 = myUB / s;
  double s2 = myUB / s1;
  double yr = s1 > s2 ? s1 : s2;
  double yl = -((-myLB) / yr);
  Parameter y(yl, yr);
  if (l == sentinel)
    y.increasePrecision();
  Parameter yPrev;
  do {
    yPrev = y;
    Parameter m = y.midP();
    Parameter z = m - (m * m - *this) / y / 2;
    y = yPrev.intersect(z);
  } while (y.subset(yPrev));
  return y;
}

}

