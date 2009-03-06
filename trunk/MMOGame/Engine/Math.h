#pragma once

class Vector {
public:
	Vector() {
	}
	Vector(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	float x, y, z;
};

extern float VectorDistance(const Vector& vecA, const Vector& vecB);
extern void VectorNormalize(const Vector& vecIn, Vector& vecOut);
