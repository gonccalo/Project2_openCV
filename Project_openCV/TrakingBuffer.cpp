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

void TrakingBuffer::clear()
{
	p[(index - 1) & 0x1F] = NULL;
}
