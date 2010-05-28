#pragma once

class XUIWidget;
class TiXmlElement;

class XUIControlFactroy
{
public:
	static XUIControlFactroy* Get(const char* pClassName);

	XUIControlFactroy(const char* pClassName);
	virtual ~XUIControlFactroy();

	virtual XUIWidget* Create(TiXmlElement* pElement) = 0;
	virtual XUIWidget* Create(const char* pName) = 0;
};
