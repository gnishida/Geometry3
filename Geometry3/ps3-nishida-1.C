#include <vector>
#include <iostream>
#include "acp.h"
#include "hull3.h"

using namespace std;

/**
 * Problem 2.
 * This program reads 3D points and computes a 3D convex hull.
 */
int main(int argc, char *argv[]) {
	Parameter::enable();

	// read input data from the standard input
	int numPoints;
	std::cin >> numPoints;

	Points points;
	vector<int> hull;

	for (int i = 0; i < numPoints; ++i) {
		double x, y, z;
		std::cin >> x >> y >> z;

		points.push_back(new InputPoint(x, y, z));
	}

	convexHull3(points, hull);

	// show the results
	std::cout << hull.size() / 3 << " ";
	for (int i = 0; i < hull.size(); ++i) {
		std::cout << hull[i] << " ";
	}

	return 0;
}
