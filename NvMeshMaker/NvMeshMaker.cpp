#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "..\NvMeshLib\MeshLoaderObj.h"

static MeshLoaderObj In;
static MeshLoaderObj Out;
static float fLeft, fTop, fRight, fBottom;
static float fScale;

class CPoint {
public:
	CPoint() { }
	CPoint(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	CPoint(const CPoint& p) { x = p.x; y = p.y; z = p.z; }
	float x, y, z;
};

static bool InRect(const CPoint& p);
static void CalcBreak(const CPoint& p1, const CPoint& p2, CPoint& r);
static void CalcNormal(const CPoint& p0, const CPoint& p1, const CPoint& p2, CPoint& n);

#define F_EPSILON	0.0001

int main(int argc, char* argv[])
{
	if(argc!=5 || 4!=sscanf(argv[3], "%f,%f,%f,%f", &fLeft, &fTop, &fRight, &fBottom) || 1!=sscanf(argv[4], "%f", &fScale)) {
		printf("invalid argement!\n");
		return -1;
	}

	In.setScale(fScale);
	if(!In.load(argv[1])) {
		printf("can't open %s\n", argv[1]);
		return -1;
	}

	int vcap = 0;
	int tcap = 0;
	for(int t=0; t<In.getTriangleCount(); t++) {
		int nv1, nv2, nv3;
		In.getTriangle(t, nv1, nv2, nv3);
		CPoint v1, v2, v3;
		In.getVertex(nv1, v1.x, v1.y, v1.z);
		In.getVertex(nv2, v2.x, v2.y, v2.z);
		In.getVertex(nv3, v3.x, v3.y, v3.z);
		int mask = 0;
		if(InRect(v1)) mask |= 0x001;
		if(InRect(v2)) mask |= 0x010;
		if(InRect(v3)) mask |= 0x100;
		CPoint p1, p2, p3;
		CPoint n1, n2;

		switch(mask) {
		case 0x111:
#if 1
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(v1.x, v1.y, v1.z, vcap);
			Out.addVertex(v2.x, v2.y, v2.z, vcap);
			Out.addVertex(v3.x, v3.y, v3.z, vcap);
#endif
			continue;
		case 0x000:
			continue;
		case 0x001:
#if 1
			CalcBreak(v2, v1, p2);
			CalcBreak(v3, v1, p3);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(v1.x, v1.y, v1.z, vcap);
			Out.addVertex(p2.x, p2.y, p2.z, vcap);
			Out.addVertex(p3.x, p3.y, p3.z, vcap);
#endif
			CalcNormal(v1, v2, v3, n1);
			CalcNormal(v1, p2, p3, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
			continue;
		case 0x010:
#if 1
			CalcBreak(v1, v2, p1);
			CalcBreak(v3, v2, p3);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p1.x, p1.y, p1.z, vcap);
			Out.addVertex(v2.x, v2.y, v2.z, vcap);
			Out.addVertex(p3.x, p3.y, p3.z, vcap);
#endif
			CalcNormal(v1, v2, v3, n1);
			CalcNormal(p1, v2, p3, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
			continue;
		case 0x100:
#if 1
			CalcBreak(v1, v3, p1);
			CalcBreak(v2, v3, p2);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p1.x, p1.y, p1.z, vcap);
			Out.addVertex(p2.x, p2.y, p2.z, vcap);
			Out.addVertex(v3.x, v3.y, v3.z, vcap);
#endif
			CalcNormal(v1, v2, v3, n1);
			CalcNormal(p1, p2, v3, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
			continue;
		case 0x110:
#if 1
			CalcBreak(v1, v2, p2);
			CalcBreak(v1, v3, p3);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p3.x, p3.y, p3.z, vcap);
			Out.addVertex(p2.x, p2.y, p2.z, vcap);
			Out.addVertex(v2.x, v2.y, v2.z, vcap);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p3.x, p3.y, p3.z, vcap);
			Out.addVertex(v2.x, v2.y, v2.z, vcap);
			Out.addVertex(v3.x, v3.y, v3.z, vcap);
			CalcNormal(v1, v2, v3, n1);
			CalcNormal(p3, p2, v2, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
			CalcNormal(p3, v2, v3, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
#endif
			continue;
		case 0x101:
#if 1
			CalcBreak(v2, v1, p1);
			CalcBreak(v2, v3, p3);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p1.x, p1.y, p1.z, vcap);
			Out.addVertex(p3.x, p3.y, p3.z, vcap);
			Out.addVertex(v3.x, v3.y, v3.z, vcap);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p1.x, p1.y, p1.z, vcap);
			Out.addVertex(v3.x, v3.y, v3.z, vcap);
			Out.addVertex(v1.x, v1.y, v1.z, vcap);
			CalcNormal(v1, v2, v3, n1);
			CalcNormal(p1, p3, v3, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
			CalcNormal(p1, v3, v1, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
#endif
			continue;
		case 0x011:
#if 1
			CalcBreak(v3, v1, p1);
			CalcBreak(v3, v2, p2);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p2.x, p2.y, p2.z, vcap);
			Out.addVertex(p1.x, p1.y, p1.z, vcap);
			Out.addVertex(v1.x, v1.y, v1.z, vcap);
			Out.addTriangle(Out.getVertCount()+1, Out.getVertCount()+2, Out.getVertCount()+3, tcap);
			Out.addVertex(p2.x, p2.y, p2.z, vcap);
			Out.addVertex(v1.x, v1.y, v1.z, vcap);
			Out.addVertex(v2.x, v2.y, v2.z, vcap);
			CalcNormal(v1, v2, v2, n1);
			CalcNormal(p2, p1, v1, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
			CalcNormal(p2, v1, v2, n2);
			assert(abs(n1.x-n2.x)<F_EPSILON && abs(n1.y-n2.y)<F_EPSILON && abs(n1.z-n2.z)<F_EPSILON);
#endif
			continue;
		default:
			assert(0);
			printf("very error!\n");
			return -1;
		}
	}

	if(!Out.save(argv[2])) {
		printf("can't save %s\n", argv[2]);
		return -1;
	}

	return 0;
}

bool InRect(const CPoint& p)
{
	if(p.x<fLeft || p.x>fRight) return false;
	if(p.y<fTop || p.y>fBottom) return false;
	return true;
}

static void CalcBreakL(const CPoint& w, const CPoint& l, CPoint& r)
{
	assert(w.x<fLeft);
	float fV = (fLeft - w.x) / (l.x - w.x);
	r.x = fLeft;
	r.y = w.y + (l.y - w.y) * fV;
	r.z = w.z + (l.z - w.z) * fV;
}

static void CalcBreakR(const CPoint& w, const CPoint& l, CPoint& r)
{
	assert(w.x>fRight);
	float fV = (w.x - fRight) / (w.x - l.x);
	r.x = fRight;
	r.y = w.y - (w.y - l.y) * fV;
	r.z = w.z - (w.z - l.z) * fV;
}

static void CalcBreakT(const CPoint& w, const CPoint& l, CPoint& r)
{
	assert(w.y<fTop);
	float fV = (fTop - w.y) / (l.y - w.y);
	r.y = fTop;
	r.x = w.x + (l.x - w.x) * fV;
	r.z = w.z + (l.z - w.z) * fV;
}

static void CalcBreakB(const CPoint& w, const CPoint& l, CPoint& r)
{
	assert(w.y>fBottom);
	float fV = (w.y - fBottom) / (w.y - l.y);
	r.y = fBottom;
	r.x = w.x - (w.x - l.x) * fV;
	r.z = w.z - (w.z - l.z) * fV;
}

void CalcBreak(const CPoint& w, const CPoint& l, CPoint& r)
{
	assert(!InRect(w) && InRect(l));
	int mask = 0;
	if(w.x<fLeft) mask |= 0x01;
	if(w.x>fRight) mask |= 0x02;
	if(w.y<fTop) mask |= 0x10;
	if(w.y>fBottom) mask |= 0x20;

	CPoint p1, p2;
	switch(mask) {
	case 0x01:
		CalcBreakL(w, l, r);
		return;
	case 0x02:
		CalcBreakR(w, l, r);
		return;
	case 0x10:
		CalcBreakT(w, l, r);
		return;
	case 0x20:
		CalcBreakB(w, l, r);
		return;
	case 0x11:
		CalcBreakL(w, l, p1);
		CalcBreakT(w, l, p2);
		return;
	case 0x22:
		CalcBreakR(w, l, p1);
		CalcBreakB(w, l, p2);
		return;
	case 0x21:
		CalcBreakL(w, l, p1);
		CalcBreakB(w, l, p2);
		return;
	case 0x12:
		CalcBreakR(w, l, p1);
		CalcBreakT(w, l, p2);
		return;
	}

	assert(		(InRect(p1) && !InRect(p2))
			||	(!InRect(p1) && InRect(p2)) );

	if(InRect(p1)) { r = p1; assert(!InRect(p2)); }
	else {
		if(InRect(p2)) { r = p2; assert(!InRect(p1)); }
		else assert(0);
	}
}

void CalcNormal(const CPoint& p0, const CPoint& p1, const CPoint& p2, CPoint& n)
{
	CPoint e0(p1.x-p0.x, p1.y-p0.y, p1.z-p0.z);
	CPoint e1(p2.x-p0.x, p2.y-p0.y, p2.z-p0.z);
	n.x = e0.y*e1.z - e0.z*e1.y;
	n.y = e0.z*e1.x - e0.x*e1.z;
	n.z = e0.x*e1.y - e0.y*e1.x;
	float d = sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
	if (d > 0) {
		d = 1.0f/d;
		n.x *= d;
		n.y *= d;
		n.z *= d;
	}
}
