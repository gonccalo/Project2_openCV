#include "TrakingBuffer.h"

bool TrakingBuffer::put(Point  *point)
{
	p[index] = new Point(*point);
	index = (index+1) & 0x1F;
	return true;
}

Point* TrakingBuffer::get(uint j)
{
	return p[(index-(j+1)) & 0x1F];
}

Mat TrakingBuffer::toMat()
{
	Mat result = Mat();
	for (int i = 0; i < 32; i++) {
		if (p[i] == NULL)
			return result;
		result.push_back(p[i]);
	}
}
