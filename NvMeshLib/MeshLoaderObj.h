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

#ifndef MESHLOADER_OBJ
#define MESHLOADER_OBJ

class MeshLoaderObj
{
public:
	MeshLoaderObj();
	~MeshLoaderObj();

	bool load(const char* fileName);
	bool save(const char* filename);
	void clear();

	inline const float* getVerts() const { return m_verts; }
	inline const float* getNormals() const { return m_normals; }
	inline const int* getTris() const { return m_tris; }
	inline int getVertCount() const { return m_vertCount; }
	inline int getTriCount() const { return m_triCount; }

	void getVertex(int ndx, float& x, float& y, float& z);
	void getTriangle(int ndx, int& v1, int& v2, int& v3);

	int getVertexCount() { return m_vertCount; }
	int getTriangleCount() { return m_triCount; }

	void addVertex(float x, float y, float z, int& cap);
	void addTriangle(int a, int b, int c, int& cap);

	void setScale(float scale) { m_scale = scale; }
	float getScale() { return m_scale; }

//private:
public:

	float* m_verts;
	int* m_tris;
	float* m_normals;
	int m_vertCount;
	int m_triCount;
	float m_scale;
};

#endif // MESHLOADER_OBJ