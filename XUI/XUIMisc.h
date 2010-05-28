#pragma once

#include <string>
typedef std::string XUIString;

#include <list>
#include <map>
#include <set>

class XUIPoint
{
public:
	XUIPoint() { }
	XUIPoint(int _nX, int _nY) {
		x = _nX;
		y = _nY;
	}
	XUIPoint(const XUIPoint& Point) {
		x = Point.x;
		y = Point.y;
	}

	bool operator ==(const XUIPoint& point) const
	{
		return (x == point.x && y == point.y);
	}

	bool operator !=(const XUIPoint& point) const
	{
		return (x != point.x || y != point.y);
	}

	int x, y;
};

namespace XUI_INPUT {
	static const unsigned short MOUSE_LBUTTON = 0x1;
	static const unsigned short MOUSE_RBUTTON = 0x2;
	static const unsigned short MOUSE_MBUTTON = 0x4;
};

class XUIRect
{
public:
	XUIRect() {
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
	}

	XUIRect(int l, int t, int r, int b) {
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	XUIRect(const XUIPoint& point, int width, int height) {
		right = (left = point.x) + width;
		bottom = (top = point.y) + height;
	}

	XUIRect(const XUIPoint& topLeft, const XUIPoint& bottomRight) {
		left = topLeft.x;
		top = topLeft.y;
		right = bottomRight.x;
		bottom = bottomRight.y;
	}

	int Width() const {
		return right - left;
	}

	int Height() const {
		return bottom - top;
	}

	XUIPoint& TopLeft()
	{
		return *((XUIPoint*)this);
	}

	XUIPoint& BottomRight()
	{
		return *((XUIPoint*)this + 1);
	}

	const XUIPoint& TopLeft() const {
		return *((XUIPoint*)this);
	}

	const XUIPoint& BottomRight() const {
		return *((XUIPoint*)this + 1);
	}

	bool PointInRect(const XUIPoint& point) const {
		return point.x>=left && point.x<right && point.y>=top && point.y<bottom;
	}

	void SetRect(int x1, int y1, int x2, int y2) {
		left = x1;
		top = y1;
		right = x2;
		bottom = y2;
	}

	void SetRect(const XUIPoint& topLeft, const XUIPoint& bottomRight) {
		SetRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
	}

	int left, top, right, bottom;
};
