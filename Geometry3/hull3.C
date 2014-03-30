#include "hull3.h"
#include "harrange.h"

void convexHull3 (Points &points, Points &hull)
{
	Arrangement arr;

	init(arr, points);


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
	Vertex* v1 = arr.addVertex(p1);
	Vertex* v2 = arr.addVertex(p1);
	Vertex* v3 = arr.addVertex(p1);
	Vertex* v4 = arr.addVertex(p1);

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
	
	e1->next = e2; e1->twin = e7;
	e2->next = e5; e2->twin = e8;
	e3->next = e12; e3->twin = e9;
	e4->next = e9; e4->twin = e10;
	e5->next = e7; e5->twin = e11;
	e6->next = e10;
	e7->next = e2;
	e8->next = e3;
	e9->next = e1;
	e10->next = e11;
	e11->next = e6;
	e12->next = e8;

	arr.formFaces();
}


