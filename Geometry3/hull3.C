#include "hull3.h"
#include <fstream>

int Orient3D::sign ()
{
  PV3 u = d->getP() - a->getP(), v = b->getP() - a->getP(), 
    w = c->getP() - a->getP();
  return u.tripleProduct(v, w).sign();
}

void convexHull3 (Points &points, Points &hull)
{
	Arrangement arr;

	init(arr, points);

	debug(arr);
}

/**
 * Find four points p1, p2, p3, p4 in P that form a tetrahedron, and build a tetrahedron.
 */
void init (Arrangement &arr, Points &points)
{
	buildTetrahedron (arr, points[0], points[1], points[2], points[3]);

	// permute the remained points

	// Initialize the conflict graphwith all visible pairs(pt,f), where f is a facet of C and t > 4. 

	for (int r = 5; r < points.size(); ++r) {
		// Fconflict(Pr) is empty (that is, Pr lies inside C), nothing changes.


	}
}

/**
 * Build a tetrahedron that consists of four points.
 */
void buildTetrahedron (Arrangement &arr, Point* p1, Point* p2, Point* p3, Point* p4)
{
	cout << "build tetrahedron" << endl;

	Vertex* v1 = arr.addVertex(p1);
	Vertex* v2 = arr.addVertex(p2);
	Vertex* v3 = arr.addVertex(p3);
	Vertex* v4 = arr.addVertex(p4);

	Edge* e1 = arr.addHalfEdge(v1, NULL, NULL, true, false);
	Edge* e2 = arr.addHalfEdge(v2, NULL, NULL, true, false);
	Edge* e3 = arr.addHalfEdge(v3, NULL, NULL, true, false);
	Edge* e4 = arr.addHalfEdge(v1, NULL, NULL, true, false);
	Edge* e5 = arr.addHalfEdge(v2, NULL, NULL, true, false);
	Edge* e6 = arr.addHalfEdge(v4, NULL, NULL, true, false);
	Edge* e7 = arr.addHalfEdge(v2, NULL, NULL, true, false);
	Edge* e8 = arr.addHalfEdge(v3, NULL, NULL, true, false);
	Edge* e9 = arr.addHalfEdge(v1, NULL, NULL, true, false);
	Edge* e10 = arr.addHalfEdge(v4, NULL, NULL, true, false);
	Edge* e11 = arr.addHalfEdge(v4, NULL, NULL, true, false);
	Edge* e12 = arr.addHalfEdge(v3, NULL, NULL, true, false);
	
	cout << "add half edge done." << endl;

	e1->next = e4; e1->twin = e7;
	e2->next = e5; e2->twin = e8;
	e3->next = e12; e3->twin = e9;
	e4->next = e9; e4->twin = e10;
	e5->next = e7; e5->twin = e11;
	e6->next = e10; e6->twin = e12;
	e7->next = e2; e7->twin = e1;
	e8->next = e3; e8->twin = e2;
	e9->next = e1; e9->twin = e3;
	e10->next = e11; e10->twin = e4;
	e11->next = e6; e11->twin = e5;
	e12->next = e8; e12->twin = e6;

	cout << "edge built." << endl;

	arr.formFaces();

	cout << "form faces done." << endl;
}


//////////////////////////////////////////////////////////////////////////////////
// arrangement

bool Edge::incident (Edge *e) const
{
  return tail == e->tail || tail == e->head() ||
    head() == e->tail || head() == e->head();
}

Edge * Edge::formLoop ()
{
  Edge *e = this, *l = this;
  while (true) {
    e->flag = true;
    e = e->twin->next;
    if (e->flag)
      break;
  }

  return l;
}

Arrangement::~Arrangement ()
{
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
  delete e->twin;
  delete e;
}

void Arrangement::formFaces ()
{
	for (Faces::iterator f = faces.begin(); f != faces.end(); ++f)
		delete *f;
	faces.clear();

	cout << "faces cleared." << endl;

	Edges inner;
	for (Edges::iterator e = edges.begin(); e != edges.end(); ++e) {
		cout << "edge:" << endl;
		pe(*e);

		if (!(*e)->flag) {
			cout << "  not visited => form loop." << endl;
			Edge *l = (*e)->formLoop();

			cout << "  OK." << endl;

			Face *f = new Face;
			faces.push_back(f);
			addBoundary(l, f);

			cout << "  added boundary." << endl;
		}
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

//////////////////////////////////////////////////////////////////////////////////
// debug
void debug (Arrangement &arr)
{
	Points points;
	vector<int> data;

	std::ofstream ofs("test.vtk");

	for (Faces::iterator f = arr.faces.begin(); f != arr.faces.end(); ++f) {
		Edge *e = (*f)->boundary[0];
		Edge *g = e;

		do {
			Point* p = g->tail->p;

			int index = -1;

			for (int i = 0; i < points.size(); ++i) {
				if (points[i] == p) {
					index = i;
				}
			}

			if (index == -1) {
				points.push_back(p);
				index = points.size() - 1;
			}

			data.push_back(index);

			g = g->twin->next;
		} while (g != e);
	}

	outputVTK(points, data, ofs);
}

void outputVTK (const Points &pts, const vector<int> &data, ostream &ostr)
{
  int np = pts.size(), nt = data.size()/3;
  ostr << setprecision(16) << "# vtk DataFile Version 3.0" << endl
       << "vtk output" << endl << "ASCII" << endl
       << "DATASET POLYDATA" << endl 
       << "POINTS " << np << " double" << endl;
  for (Points::const_iterator p = pts.begin(); p != pts.end(); ++p) {
    const PV3 &q = (*p)->getP();
    ostr << q.x.mid() << " " << q.y.mid() << " " << q.z.mid() << endl;
  }
  ostr << endl << "POLYGONS " << nt << " " << 4*nt << endl;
  for (int i = 0; i < nt; ++i)
    ostr << "3 " << data[3*i] << " " << data[3*i+1] << " "
	 << data[3*i+2] << endl;
  ostr << endl << "CELL_DATA " << nt << endl 
       << "POINT_DATA " << np << endl;
}

void pp (Point *p)
{
  PV3 pp = p->getP();
  cerr << setprecision(16) << "(" << pp.x.mid() << " " << pp.y.mid() << " " << pp.z.mid() << ")" << endl;
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