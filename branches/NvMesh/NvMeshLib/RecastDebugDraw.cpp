//
// Copyright (c) 2009 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#define _USE_MATH_DEFINES
#include <math.h>
#include "RecastDebugDraw.h"
#include <windows.h>
#include <gl\glew.h>

template<class T> inline T rcMin(T a, T b) { return a < b ? a : b; }
template<class T> inline T rcMax(T a, T b) { return a > b ? a : b; }

inline void vmin(float* mn, const float* v)
{
	mn[0] = rcMin(mn[0], v[0]);
	mn[1] = rcMin(mn[1], v[1]);
	mn[2] = rcMin(mn[2], v[2]);
}

inline void vmax(float* mx, const float* v)
{
	mx[0] = rcMax(mx[0], v[0]);
	mx[1] = rcMax(mx[1], v[1]);
	mx[2] = rcMax(mx[2], v[2]);
}

inline void vcopy(float* dest, const float* v)
{
	dest[0] = v[0];
	dest[1] = v[1];
	dest[2] = v[2];
}

void rcCalcBounds(const float* verts, int nv, float* bmin, float* bmax)
{
	// Calculate bounding box.
	vcopy(bmin, verts);
	vcopy(bmax, verts);
	for (int i = 1; i < nv; ++i)
	{
		const float* v = &verts[i*3];
		vmin(bmin, v);
		vmax(bmax, v);
	}
}

void rcDebugDrawMesh(const float* verts, int nverts,
					 const int* tris, const float* normals, int ntris,
					 const unsigned char* flags)
{	
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < ntris*3; i += 3)
	{
		float a = (2+normals[i+0]+normals[i+1])/4;
		if (flags && !flags[i/3])
			glColor3f(a,a*0.3f,a*0.1f);
		else
			glColor3f(a,a,a);
		glVertex3fv(&verts[tris[i]*3]);
		glVertex3fv(&verts[tris[i+1]*3]);
		glVertex3fv(&verts[tris[i+2]*3]);
	}
	glEnd();
}

void rcDebugDrawMeshSlope(const float* verts, int nverts,
						  const int* tris, const float* normals, int ntris,
						  const float walkableSlopeAngle)
{
	const float walkableThr = cosf(walkableSlopeAngle/180.0f*(float)M_PI);
	
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < ntris*3; i += 3)
	{
		const float* norm = &normals[i];
		float a = (2+norm[0]+norm[1])/4;
		if (norm[1] > walkableThr)
			glColor3f(a,a,a);
		else
			glColor3f(a,a*0.3f,a*0.1f);
		glVertex3fv(&verts[tris[i]*3]);
		glVertex3fv(&verts[tris[i+1]*3]);
		glVertex3fv(&verts[tris[i+2]*3]);
	}
	glEnd();
}
