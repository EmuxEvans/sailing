#pragma once

class XUIScriptParam
{
};

class XUIScriptProperty
{
public:
	virtual ~XUIScriptProperty() { }

	virtual void Setter(XUIScriptParamObject* pObject, const XUIScriptParam* pParam) = 0;
	virtual XUIScriptParam* Getter(XUIScriptParamObject* pObject) = 0;
};

class XUIScriptMethod
{
public:
	virtual ~XUIScriptMethod() { }

	virtual XUIScriptParam* Call(std::vector<XUIScriptParam*> Params);
};

class XUIScriptTemplate
{
public:
	XUIScriptTemplate();
	~XUIScriptTemplate();

	void InsertProperty();
	void DeleteProperty();
	int GetPropertyCount();
	XUIScriptProperty* GetProperty(int nIndex);
	XUIScriptProperty* GetProperty(const char* pName);

	void InsertMethod();
	void RemoveMethod();
	int GetMethodCount();
	XUIScriptMethod* GetMethod(int nIndex);
	XUIScriptMethod* GetMethod(const char* pName);

};
