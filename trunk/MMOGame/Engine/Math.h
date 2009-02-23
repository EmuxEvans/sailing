#pragma once

typedef struct Vector {
	float		x;
	float		y;
	float		z;
} Vector;

float VectorDistance(const Vector& vecA, const Vector& vecB);
void VectorNormalize(const Vector& vecIn, Vector& vecOut);
