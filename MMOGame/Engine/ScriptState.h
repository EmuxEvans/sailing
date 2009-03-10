#pragma once

class IScriptState
{
public:
	virtual ~IScriptState() {}

	virtual void Release() = 0;

	virtual bool InitState() = 0;
	virtual bool ClearState() = 0;
	virtual bool ResetState() = 0;

	virtual bool LoadScript(const char* filename) = 0;

	virtual bool Call(const char* funcname) = 0;
	virtual bool Call(const char* funcname, void* obj, const char* obj_type, unsigned int cmd, unsigned int who, void* data, const char* data_type) = 0;
};

IScriptState* CreateScriptState();
