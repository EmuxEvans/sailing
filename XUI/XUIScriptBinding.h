#pragma once

class XUIScriptProperty
{
};

class XUIScriptMethod
{
};

class XUIScriptParam
{
};

class XUIScriptBinding
{
public:
	XUIScriptBinding();
	~XUIScriptBinding();

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
