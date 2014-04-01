#include <vector>
#include <iostream>
#include "acp.h"
#include "hull3.h"

using namespace std;

/**
 * Problem 3.
 * This program reads 2D points and computes a delaunay triangulation by using 3D convex hull function.
 */
int main(int argc, char *argv[]) {
	Parameter::enable();

	// read input data from the standard input
	int numPoints;
	std::cin >> numPoints;

	Points2D points;
	vector<int> hull;

	for (int i = 0; i < numPoints; ++i) {
		double x, y;
		std::cin >> x >> y;

		points.push_back(new InputPoint2D(x, y));
	}

	vector<int> triangles;
	triangulate(points, triangles);

	// show the results
	std::cout << triangles.size() / 3 << " ";
	for (int i = 0; i < triangles.size(); ++i) {
		std::cout << triangles[i] << " ";
	}

	return 0;
}
