#include <stddef.h>

#include "TinyXML.h"
#include "XmlUtils.h"

TiXmlElement* XmlUtils::ToElement(TiXmlNode* pNode)
{
	return pNode?pNode->ToElement():NULL;
}

TiXmlElement* XmlUtils::GetElement(TiXmlNode* pNode, const char* name)
{
	TiXmlElement* pElement = ToElement(pNode);
	if(!pElement) return NULL;
	return ToElement(pElement->FirstChild(name));
}

const char* XmlUtils::GetText(TiXmlNode* pNode)
{
	if(pNode==NULL) return NULL;
	TiXmlElement* pElement = ToElement(pNode);
	if(pElement==NULL) return NULL;
	return pElement->GetText();
}

const char* XmlUtils::GetText(TiXmlElement* pElement, const char* name)
{
	return pElement?GetText(ToElement(pElement->FirstChild(name))):NULL;
}

const char* XmlUtils::GetAttribute(TiXmlElement* pElement, const char* name)
{
	return pElement?pElement->Attribute(name):NULL;
}
