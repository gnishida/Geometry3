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

#ifndef OBJECT_H
#define OBJECT_H

#include "pv.h"
#include <vector>

namespace acp {

// List of parameters.
class Parameters {
  std::vector <Parameter *> parameters;

public:
  Parameters () : parameters(0) {}
  Parameters (Parameter &p) : parameters(1) { parameters[0] = &p; }
  Parameters (PV2 &v) : parameters(2) { parameters[0] = &v.x; parameters[1] = &v.y; }
  Parameters (PV3 &v) : parameters(3) { parameters[0] = &v.x; parameters[1] = &v.y; parameters[2] = &v.z; }
  Parameters (Parameter &p, PV2 &v) : parameters(3) { parameters[0] = &p; parameters[1] = &v.x; parameters[2] = &v.y; }
  Parameters (PV3 &v, Parameter &p) : parameters(4) { parameters[0] = &v.x; parameters[1] = &v.y; parameters[2] = &v.z; parameters[3] = &p; }

  Parameters add (Parameter &p) { parameters.push_back(&p); return *this; }
  Parameters add (PV2 &v) { parameters.push_back(&v.x); parameters.push_back(&v.y); return *this; }
  Parameters add (PV3 &v) { parameters.push_back(&v.x); parameters.push_back(&v.y); parameters.push_back(&v.z); return *this; }

  int size () { return parameters.size(); }
  Parameter *get (int i) { return parameters[i]; }
};

class Object;

// List of objects.
class Objects {
  std::vector<Object *> objects;

public:
  Objects () : objects(0) {}
  Objects (Object *o0) : objects(1) { objects[0] = o0; }
  Objects (Object *o0, Object *o1) : objects(2) { objects[0] = o0; objects[1] = o1; }
  Objects (Object *o0, Object *o1, Object *o2) : objects(3) { objects[0] = o0; objects[1] = o1; objects[2] = o2; }
  Objects (Object *o0, Object *o1, Object *o2, Object *o3) : objects(4) { objects[0] = o0; objects[1] = o1; objects[2] = o2; objects[3] = o3; }
  Objects (Object *o0, Object *o1, Object *o2, Object *o3, Object *o4) : objects(5) { objects[0] = o0; objects[1] = o1; objects[2] = o2; objects[3] = o3; objects[4] = o4; }
  Objects (Object *o0, Object *o1, Object *o2, Object *o3, Object *o4, Object *o5) : objects(6) { objects[0] = o0; objects[1] = o1; objects[2] = o2; objects[3] = o3; objects[4] = o4; objects[5] = o5; }
  Objects (Object *o0, Object *o1, Object *o2, Object *o3, Object *o4, Object *o5, Object *o6) : objects(7) { objects[0] = o0; objects[1] = o1; objects[2] = o2; objects[3] = o3; objects[4] = o4; objects[5] = o5; objects[6] = o6; }
  Objects (Object *o0, Object *o1, Object *o2, Object *o3, Object *o4, Object *o5, Object *o6, Object *o7) : objects(8) { objects[0] = o0; objects[1] = o1; objects[2] = o2; objects[3] = o3; objects[4] = o4; objects[5] = o5; objects[6] = o6; objects[7] = o7; }
  Objects (Object *o0, Object *o1, Object *o2, Object *o3, Object *o4, Object *o5, Object *o6, Object *o7, Object *o8) : objects(9) { objects[0] = o0; objects[1] = o1; objects[2] = o2; objects[3] = o3; objects[4] = o4; objects[5] = o5; objects[6] = o6; objects[7] = o7; objects[8] = o8; }
  Objects (Object *o0, Object *o1, Object *o2, Object *o3, Object *o4, Object *o5, Object *o6, Object *o7, Object *o8, Object *o9) : objects(10) { objects[0] = o0; objects[1] = o1; objects[2] = o2; objects[3] = o3; objects[4] = o4; objects[5] = o5; objects[6] = o6; objects[7] = o7; objects[8] = o8; objects[9] = o9; }

  void add (Object *o) { objects.push_back(o); }

  int size () { return objects.size(); }
  Object *get (int i) { return objects[i]; }
};

class Predicate;
class AnglePoly;

// Use this as the parent class for all your geometric objects:
// points, lines, etc.
// Your class must define getParameters(), getObjects(), and calculate().
class Object {
public:
  Object () {}

  // Parameters defined in this Object.
  virtual Parameters getParameters () = 0;

  // Objects on which this Object depends.
  virtual Objects getObjects () = 0;

  // Calculate the Object parameters from its objects.
  // The derived class should define calculate() unless it has no
  // parameters to calculate or it does not depend on any objects.
  virtual void calculate () {
    assert(getObjects().size() == 0 || getParameters().size() == 0);
  }

  friend class Predicate;
  friend class AnglePoly;
private:
  // If Parameter has multiple levels of precision, this
  // increasePrecision should take the desired level of precision as
  // an argument.
  void increasePrecision () {
    Parameters parameters = getParameters();
    if (parameters.size() > 0 && parameters.get(0)->increased())
      return;
    for (int i = 0; i < parameters.size(); i++)
      parameters.get(i)->increasePrecision();
    Objects objects = getObjects();
    for (int i = 0; i < objects.size(); i++)
      objects.get(i)->increasePrecision();
    calculate();
  }

  void decreasePrecision () {
    Parameters parameters = getParameters();
    if (parameters.size() > 0 && parameters.get(0)->decreased())
      return;
    Objects objects = getObjects();
    for (int i = 0; i < objects.size(); i++)
      objects.get(i)->decreasePrecision();
    for (int i = 0; i < parameters.size(); i++)
      parameters.get(i)->decreasePrecision();
  }
};

// All predicates should be derived from the Predicate class
// and define the getObjects() method and sign() method.
// Use it like this:
// if (PointLineSide(p, l) < 0) ...
class Predicate {
  // Calculate the sign from the objects.
  virtual int sign () = 0;

public:
  // Objects on which this Predicate depends.
  virtual Objects getObjects () = 0;

  operator int () {
    try {
      return sign();
    } catch (SignException se) {
      Objects objects = getObjects();
      for (int i = 0; i < objects.size(); i++)
        objects.get(i)->increasePrecision();
      int ret = operator int();
      for (int i = 0; i < objects.size(); i++)
        objects.get(i)->decreasePrecision();
      return ret;
    }
  }
};  

}

// Macro to create one-argument predicate.
#define Predicate1(P, t1, v1)                            \
  class P : public acp::Predicate {                              \
    t1 v1;                                                \
  int sign ();                                                   \
  public:                                                        \
  P (t1 v1) : v1(v1) {}                           \
  acp::Objects getObjects () { return acp::Objects(v1); }    \
  };

// Macro to create two-argument predicate.
#define Predicate2(P, t1, v1, t2, v2)                            \
  class P : public acp::Predicate {                              \
    t1 v1; t2 v2;                                                \
  int sign ();                                                   \
  public:                                                        \
  P (t1 v1, t2 v2) : v1(v1), v2(v2) {}                           \
  acp::Objects getObjects () { return acp::Objects(v1, v2); }    \
  };

// Macro to create three-argument predicate.
#define Predicate3(P, t1, v1, t2, v2, t3, v3)                       \
  class P : public acp::Predicate {                                 \
    t1 v1; t2 v2; t3 v3;                                            \
  int sign ();                                                      \
  public:                                                           \
  P (t1 v1, t2 v2, t3 v3) : v1(v1), v2(v2), v3(v3) {}               \
  acp::Objects getObjects () { return acp::Objects(v1, v2, v3); }   \
  };

// Macro to create four-argument predicate.
#define Predicate4(P, t1, v1, t2, v2, t3, v3, t4, v4)                 \
  class P : public acp::Predicate {                                   \
    t1 v1; t2 v2; t3 v3; t4 v4;                                       \
    int sign ();                                                      \
  public:                                                             \
  P (t1 v1, t2 v2, t3 v3, t4 v4) : v1(v1), v2(v2), v3(v3), v4(v4) {}  \
  acp::Objects getObjects () { return acp::Objects(v1, v2, v3, v4); } \
  };

// Macro to create five-argument predicate.
#define Predicate5(P, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5)           \
  class P : public acp::Predicate {                                     \
    t1 v1; t2 v2; t3 v3; t4 v4; t5 v5;                                  \
    int sign ();                                                        \
  public:                                                               \
  P (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5)                                 \
    : v1(v1), v2(v2), v3(v3), v4(v4), v5(v5) {}                         \
  acp::Objects getObjects () { return acp::Objects(v1, v2, v3, v4, v5); } \
  };

#endif
