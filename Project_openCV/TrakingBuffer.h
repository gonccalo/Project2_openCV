#pragma once
#include "opencv2/core/core.hpp"
using namespace cv;
using namespace std;
class TrakingBuffer {
public:
	bool put(Point *p);
	Point* get(int index);

private:
	Point *p[32];
	int index = 0;
};