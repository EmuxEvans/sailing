#pragma once

class XUIWidget;
class TiXmlElement;

class XUIControlFactroy
{
public:
	static XUIControlFactroy* Get(const char* pName);

	XUIControlFactroy(const char* pName);
	virtual ~XUIControlFactroy();

	virtual XUIWidget* New(TiXmlElement* pElement) = 0;

};
