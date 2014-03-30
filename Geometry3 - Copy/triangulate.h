#ifndef	TRIANGULATE
#define TRIANGULATE

#include "arrange.h"

void triangulate (Arrangement *arr, double *bbox);

void addBBox (Arrangement *arr, double *bbox);

void makeMonotone (Arrangement *arr);

void makeMonotoneVertex (Arrangement *arr, Sweep &sweep, Vertex *v);

void triangulateMonotone (Arrangement *arr);

void monotoneLoop (Edge *e, Vertices &u);

void triangulateLoop (const Vertices &u, Arrangement *arr);

void triangulateSame (Vertex *v, Vertices &stack, Arrangement *arr);

void triangulateOther (Vertex *v, Vertices &stack, Arrangement *arr);

bool VertexYOrder (Vertex *a, Vertex *b);

#endif
