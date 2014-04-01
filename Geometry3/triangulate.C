#include "hull3.h"
#include "triangulate.h"

void triangulate (Points2D &points, vector<int> &triangles)
{
	Points pts;
	for (int i = 0; i < points.size(); ++i) {
		double x = points[i]->getP().getX().mid();
		double y = points[i]->getP().getY().mid();
		double z = x * x + y * y;
		pts.push_back(new InputPoint(x, y, z));
	}

	vector<int> hull;

	convexHull3(pts, hull);

	for (int i = 0; i < hull.size(); i += 3) {
		Point *p0 = pts[hull[i]];
	}
}
