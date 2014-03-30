#ifndef MINDISK
#define MINDISK

#include "circle.h"
#include "permute.h"

Circle * minDisk (const Points &pts);

Circle * minDiskWithPoint (const Points &pts, int *p, int n, Point *q);

Circle * minDiskWithTwoPoints (const Points &pts, int *p, int n, Point *q1, Point *q2);

#endif
