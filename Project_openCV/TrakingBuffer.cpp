#include "TrakingBuffer.h"

bool TrakingBuffer::put(Point  *point)
{
	p[index] = new Point(*point);
	index = (index+1) & 0x1F;
	return true;
}

Point* TrakingBuffer::get(int index)
{
	return p[index];
}
