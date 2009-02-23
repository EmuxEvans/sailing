#include <math.h>

#include "Math.h"

float VectorDistance(const Vector& vecA, const Vector& vecB)
{
	Vector r;
	r.x = vecA.x - vecB.x;
	r.y = vecA.y - vecB.y;
	r.z = vecA.z - vecB.z;
	return sqrt((r.x*r.x) + (r.y*r.y) + (r.z*r.z));
}

void VectorNormalize(const Vector& vecIn, Vector& vecOut)
{
	float length = sqrt((vecIn.x * vecIn.x) + (vecIn.y * vecIn.y) + (vecIn.z * vecIn.z));
	vecOut.x = vecIn.x / length;
	vecOut.y = vecIn.y / length;
	vecOut.z = vecIn.z / length;
}
