#include "hull3.h"
#include <fstream>

int face_id = 0;
int edge_id = 0;
int vertex_id = 0;

/**
 * Check if a point d is above the triangle {a, b, c}. 
 * This can be used to also check if the triangle is visible from the point d.
 */
int Orient3D::sign ()
{
  PV3 u = d->getP() - a->getP(), v = b->getP() - a->getP(), 
    w = c->getP() - a->getP();
  return u.tripleProduct(v, w).sign();
}

void convexHull3 (Points &points, vector<int> &hull)
{
	if (points.size() < 4) {
		return;
	}

	Arrangement arr;

	// add vertices
	for (int i = 0; i < points.size(); ++i) {
		Vertex *v = arr.addVertex(points[i]);
		v->id = vertex_id++;
	}

	init(arr);

	// Compute a random permutation p5, p6, . . . , pn of the remaining points.
	int n = points.size() - 4;
	int *p = new int [n];
	randomPermutation (n, p);

	for (int i = 4; i < arr.vertices.size(); ++i) {
		int r = p[i - 4] + 4;

		// Fconflict(Pr) is empty (that is, Pr lies inside C), nothing changes.
		if (arr.vertices[r]->visibleFaces.size() == 0) continue;

		// Compute the horizon
		Edges horizon;
		findHorizon(arr, arr.vertices[r], horizon);

		// list up all the vertices that have to be tested for the conflict graph
		Vertices vertices;
		listUpdateVertices(arr, arr.vertices[r], horizon, vertices);

		// delete all the visible faces and edges except the horizon
		deleteVisibleCone(arr, arr.vertices[r]);

		// add all the outgoing edges from Pr, and all the new faces
		addCone(arr, arr.vertices[r], horizon, vertices);
	}

	// build the return values
	buildHull(arr, hull);

	// debug
	std::ofstream ofs("test.vtk");
	outputVTK(points, hull, ofs);
}

/**
 * Delaunay triangulation using the same algorithm as convexHull3.
 */
void triangulate (Points2D &points2D, vector<int> &triangles)
{
	if (points2D.size() < 4) {
		triangles.push_back(0);
		triangles.push_back(1);
		triangles.push_back(2);
		return;
	}

	Arrangement arr;

	// arrange the 3D points
	Points points;
	for (int i = 0; i < points2D.size(); ++i) {
		double x = points2D[i]->getP().getX().mid();
		double y = points2D[i]->getP().getY().mid();
		double z = x * x + y * y;
		points.push_back(new InputPoint(x * 2, y * 2, z));
	}

	// add vertices
	for (int i = 0; i < points.size(); ++i) {
		Vertex *v = arr.addVertex(points[i]);
		v->id = vertex_id++;
	}

	init(arr);

	// Compute a random permutation p5, p6, . . . , pn of the remaining points.
	int n = points.size() - 4;
	int *p = new int [n];
	randomPermutation (n, p);

	for (int i = 4; i < arr.vertices.size(); ++i) {
		int r = p[i - 4] + 4;

		// Fconflict(Pr) is empty (that is, Pr lies inside C), nothing changes.
		if (arr.vertices[r]->visibleFaces.size() == 0) continue;

		// compute the horizon
		Edges horizon;
		findHorizon(arr, arr.vertices[r], horizon);

		// list up all the vertices that have to be tested for the conflict graph
		Vertices vertices;
		listUpdateVertices(arr, arr.vertices[r], horizon, vertices);

		// delete all the visible faces and edges except the horizon
		deleteVisibleCone(arr, arr.vertices[r]);

		// add all the outgoing edges from Pr, and all the new faces
		addCone(arr, arr.vertices[r], horizon, vertices);
	}

	// list up all the faces that are visible from (0, 0, -oo)
	for (Faces::iterator f = arr.faces.begin(); f != arr.faces.end(); ++f) {
		Vertex *v0 = (*f)->edge->tail;
		Vertex *v1 = (*f)->edge->twin->next->tail;
		Vertex *v2 = (*f)->edge->twin->next->twin->next->tail;

		// if it is not visible, then skip it.
		PV3 u = v1->p->getP() - v0->p->getP();
		PV3 v = v2->p->getP() - v0->p->getP();
		PV3 w(0, 0, -1);
		if (w.tripleProduct(u, v).sign() <= 0) continue;

		// if it is visible, then copy the indices to the result.
		// Note that since the triangle is in counter-clockwise order viewed from the bottom, 
		// we have to reverse the order to get the counter-clockwise order viewed from the top.
		triangles.push_back(v0->id);
		triangles.push_back(v2->id);
		triangles.push_back(v1->id);
	}
}

/**
 * Find four points p1, p2, p3, p4 in P that form a tetrahedron, and build a tetrahedron.
 */
void init (Arrangement &arr)
{
	if (Orient3D(arr.vertices[0]->p, arr.vertices[1]->p, arr.vertices[2]->p, arr.vertices[3]->p) > 0) {
		buildTetrahedron (arr, arr.vertices[0], arr.vertices[1], arr.vertices[2], arr.vertices[3]);
	} else {
		buildTetrahedron (arr, arr.vertices[0], arr.vertices[2], arr.vertices[1], arr.vertices[3]);
	}

	// Initialize the conflict graph with all visible pairs(pt,f), where f is a facet of C and t > 4. 
	for (int i = 4; i < arr.vertices.size(); ++i) {
		for (Faces::iterator f = arr.faces.begin(); f != arr.faces.end(); ++f) {
			// check if this face is visible from the vertex
			if ((*f)->visible(arr.vertices[i])) {
				arr.vertices[i]->visibleFaces.push_back(*f);
				(*f)->visibleVertices.push_back(arr.vertices[i]);
			}
		}
	}
}

/**
 * Build a tetrahedron that consists of four points.
 */
void buildTetrahedron (Arrangement &arr, Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4)
{
	Edge* e1 = arr.addHalfEdge(v1, NULL, NULL, false);
	Edge* e2 = arr.addHalfEdge(v2, NULL, NULL, false);
	Edge* e3 = arr.addHalfEdge(v4, NULL, NULL, false);
	Edge* e4 = arr.addHalfEdge(v2, NULL, NULL, false);
	Edge* e5 = arr.addHalfEdge(v3, NULL, NULL, false);
	Edge* e6 = arr.addHalfEdge(v3, NULL, NULL, false);
	Edge* e7 = arr.addHalfEdge(v2, NULL, NULL, false);
	Edge* e8 = arr.addHalfEdge(v4, NULL, NULL, false);
	Edge* e9 = arr.addHalfEdge(v1, NULL, NULL, false);
	Edge* e10 = arr.addHalfEdge(v3, NULL, NULL, false);
	Edge* e11 = arr.addHalfEdge(v4, NULL, NULL, false);
	Edge* e12 = arr.addHalfEdge(v1, NULL, NULL, false);

	e1->id = 0;
	e2->id = 1;
	e3->id = 2;
	e4->id = 3;
	e5->id = 4;
	e6->id = 5;
	e7->id = 6;
	e8->id = 7;
	e9->id = 8;
	e10->id = 9;
	e11->id = 10;
	e12->id = 11;
	
	e1->next = e12; e1->twin = e7;
	e2->next = e4; e2->twin = e8;
	e3->next = e11; e3->twin = e9;
	e4->next = e7; e4->twin = e10;
	e5->next = e6; e5->twin = e11;
	e6->next = e10; e6->twin = e12;
	e7->next = e2; e7->twin = e1;
	e8->next = e3; e8->twin = e2;
	e9->next = e1; e9->twin = e3;
	e10->next = e5; e10->twin = e4;
	e11->next = e8; e11->twin = e5;
	e12->next = e9; e12->twin = e6;

	arr.formFaces();
}

/**
 * Build a horizon that is a edge loop, 
 * each of which has one face visible and the other invisible from the specified vertex.
 */
void findHorizon (Arrangement &arr, Vertex *v, Edges &horizon)
{
	// find an edge that constitutes a part of the horizon
	Edge *e0 = NULL;
	for (int i = 0; i < v->visibleFaces.size() && e0 == NULL; ++i) {
		Edge *e = v->visibleFaces[i]->edge;
		Edge *g = e;

		do {
			if (!g->twin->face->visible(v)) {
				e0 = g;
				break;
			}

			g = g->twin->next;
		} while (g != e);
	}

	// traverse the next edges to find all the remaining parts of the horizon
	Edge *e = e0;
	do {
		horizon.push_back(e);

		e = e->twin->next;

		// find a part of the horizon again
		while (true) {
			if (!e->twin->face->visible(v)) {
				break;
			}

			e = e->next;
		}
	} while (e != e0);
}

/**
 * List up all the vertices that have to be tested for the conflict graph.
 */
void listUpdateVertices(Arrangement &arr, Vertex *v, Edges &horizon, Vertices &vertices)
{
	map<Vertex*, bool> hashtable;

	for (int i = 0; i < horizon.size(); ++i) {
		Face* f1 = horizon[i]->face;
		Face* f2 = horizon[i]->twin->face;

		for (int j = 0; j < f1->visibleVertices.size(); ++j) {
			if (f1->visibleVertices[j] != v) {
				hashtable[f1->visibleVertices[j]] = true;
			}
		}
		for (int j = 0; j < f2->visibleVertices.size(); ++j) {
			if (f2->visibleVertices[j] != v) {
				hashtable[f2->visibleVertices[j]] = true;
			}
		}
	}

	for (map<Vertex*, bool>::iterator it = hashtable.begin(); it != hashtable.end(); ++it) {
		vertices.push_back((*it).first);
	}
}

/**
 * Delete all the visible faces and edges except the horizon.
 */
void deleteVisibleCone(Arrangement &arr, Vertex *v)
{
	// clear the visited flag of all the edges
	for (int i = 0; i < arr.edges.size(); ++i) {
		arr.edges[i]->flag = false;
	}

	Edges toBeRemovedEdges;

	for (int i = 0; i < v->visibleFaces.size(); ++i) {
		Edge *e = v->visibleFaces[i]->edge;

		Edge *g = e;
		do {
			if (!g->flag && !g->twin->flag) {
				g->flag = true;

				if (g->twin->face->visible(v)) {
					toBeRemovedEdges.push_back(g);
				}
			}

			g = g->twin->next;
		} while (g != e);
	}

	// delete all the edges to be removed
	for (int i = 0; i < toBeRemovedEdges.size(); ++i) {
		arr.removeEdge(toBeRemovedEdges[i]);
	}

	// delete all the faces to be removed
	while (v->visibleFaces.size() > 0) {
		arr.removeFace(v->visibleFaces[0]);
	}

	v->visibleFaces.clear();
}

/**
 * Add all the outgoing edges from the vertex v to the horizon edges.
 * Also, update the conflict graph for the newly added faces.
 */
void addCone(Arrangement &arr, Vertex *v, Edges &horizon, Vertices &vertices)
{
	int n = horizon.size();

	Edges spokes1;
	Edges spokes2;
	for (int i = 0; i < n; ++i) {
		Edge *e0 = horizon[i];

		Edge *e1 = arr.addHalfEdge(e0->tail, NULL, NULL, false);
		e1->id = edge_id++;
		Edge *e2 = arr.addHalfEdge(v, NULL, NULL, false);
		e2->id = edge_id++;
		e1->twin = e2;
		e2->twin = e1;
		spokes1.push_back(e1);
		spokes2.push_back(e2);
	}

	// order the edges around the vertex, i.e. set the next pointer correctly
	for (int i = 0; i < n; ++i) {
		Edge *e = horizon[i];
		e->twin->next = spokes1[(i + 1) % n];
		spokes1[i]->next = e;
		spokes2[i]->next = spokes2[(i - 1 + n) % n];
	}

	// Add the corresponding faces
	for (int i = 0; i < n; ++i) {
		Face *f = new Face;
		arr.faces.push_back(f);
		f->id = face_id++;
		arr.addBoundary(horizon[i], f);

		// Also, update the conflict graph for added faces.
		for (int j = 0; j < vertices.size(); ++j) {
			if (f->visible(vertices[j])) {
				f->visibleVertices.push_back(vertices[j]);
				vertices[j]->visibleFaces.push_back(f);
			}
		}
	}
}

void buildHull (Arrangement &arr, vector<int> &hull)
{
	for (Faces::iterator f = arr.faces.begin(); f != arr.faces.end(); ++f) {
		Edge *e = (*f)->edge;
		Edge *g = e;

		do {
			Point* p = g->tail->p;

			int index = -1;

			for (int i = 0; i < arr.vertices.size(); ++i) {
				if (arr.vertices[i]->p == p) {
					index = i;
				}
			}

			hull.push_back(index);

			g = g->twin->next;
		} while (g != e);
	}
}

//////////////////////////////////////////////////////////////////////////////////
// arrangement

void Vertex::removeVisibleFace (Face *f) {
  int i = 0;
  while (i < visibleFaces.size())
    if (visibleFaces[i] == f) {
      visibleFaces[i] = *(visibleFaces.end()-1);
      visibleFaces.pop_back();
	  break;
    }
    else
      ++i;
}

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

/**
 * Check if this face is visible from the specified vertex.
 */
bool Face::visible(Vertex *v)
{
	Point* p0 = edge->tail->p;
	Point* p1 = edge->twin->next->tail->p;
	Point* p2 = edge->twin->next->twin->next->tail->p;

	if (Orient3D(p0, p1, p2, v->p) > 0) return true;
	else return false;
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

Edge * Arrangement::addHalfEdge (Vertex *tail, Edge *twin, Edge *next, bool flag)
{
  Edge *e = new Edge(tail, twin, next, flag);
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

void Arrangement::removeFace (Face *f)
{
  int c = 0, i = 0;
  while (i < faces.size())
    if (faces[i] == f) {
      faces[i] = *(faces.end()-1);
      faces.pop_back();
	  break;
    }
    else
      ++i;

  // update the conflict graph
  for (int i = 0; i < f->visibleVertices.size(); ++i) {
	  f->visibleVertices[i]->removeVisibleFace(f);
  }

  delete f;
}

void Arrangement::formFaces ()
{
	for (Faces::iterator f = faces.begin(); f != faces.end(); ++f)
		delete *f;
	faces.clear();

	Edges inner;
	for (Edges::iterator e = edges.begin(); e != edges.end(); ++e) {
		if (!(*e)->flag) {
			Edge *l = (*e)->formLoop();

			Face *f = new Face;
			faces.push_back(f);
			f->id = face_id++;
			addBoundary(l, f);
		}
	}
}

void Arrangement::addBoundary (Edge *e, Face *f) const
{
  f->edge = e;

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