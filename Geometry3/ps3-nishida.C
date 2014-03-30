#include <vector>
#include <iostream>
#include "acp.h"
#include "hull3.h"

using namespace std;

int main(int argc, char *argv[]) {
	Parameter::enable();

	// read input data from the standard input
	int numCircles;
	std::cin >> numCircles;

	Points points;
	Points hull;

	for (int i = 0; i < numCircles; ++i) {
		double x, y, z;
		std::cin >> x >> y >> z;

		points.push_back(new InputPoint(x, y, z));
	}

	convexHull3(points, hull);

	// show the results
	std::cout << hull.size();
	for (int i = 0; i < hull.size(); ++i) {
		std::cout << " " << hull[i]->getP().x.mid() << " " << hull[i]->getP().y.mid() << " " << hull[i]->getP().z.mid();
	}

	return 0;
}
