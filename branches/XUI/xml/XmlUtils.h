#ifndef _XMLUTILS_H_
#define _XMLUTILS_H_

namespace XmlUtils {
	TiXmlElement* ToElement(TiXmlNode* pNode);
	TiXmlElement* GetElement(TiXmlNode* pNode, const char* name);

	const char* GetText(TiXmlNode* pNode);
	const char* GetText(TiXmlElement* pElement, const char* name);

	const char* GetAttribute(TiXmlElement* pElement, const char* name);
};

#endif
