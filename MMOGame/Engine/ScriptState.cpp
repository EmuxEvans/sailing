#include <assert.h>
#include <string.h>

#include <skates/lua/lua.h>
#include <skates/lua/lauxlib.h>

#include "ScriptState.h"

class CScriptState : public IScriptState
{
public:
	CScriptState();
	virtual ~CScriptState();

	virtual void Release() {
		delete this;
	}

	virtual bool InitState();
	virtual bool ClearState();
	virtual bool ResetState();

	virtual bool LoadScript(const char* filename);

	virtual bool Call(const char* funcname);
	virtual bool Call(const char* funcname, void* obj, const char* obj_type, unsigned int cmd, unsigned int who, void* data, const char* data_type);

protected:
	void SetErrorMsg(int index);

private:
	lua_State* m_pState;
	char m_szErrorMsg[2*1024];
};


CScriptState::CScriptState()
{
	m_pState = NULL;
}

CScriptState::~CScriptState()
{
	if(m_pState) {
		ClearState();
	}
}

bool CScriptState::InitState()
{
	assert(!m_pState);

	m_pState = luaL_newstate();
	if(!m_pState) return false;

	return true;
}

bool CScriptState::ClearState()
{
	assert(m_pState);
	if(!m_pState) return false;

	lua_close(m_pState);
	m_pState = NULL;

	return true;
}

bool CScriptState::ResetState()
{
	ClearState();
	return InitState();
}

bool CScriptState::LoadScript(const char* filename)
{
	assert(m_pState);

	if(luaL_loadfile(m_pState, filename)!=0) {
		SetErrorMsg(-1);
		lua_pop(m_pState, 1);
		return false;
	}

	return true;
}

bool CScriptState::Call(const char* funcname)
{
	assert(m_pState);

	lua_getglobal(m_pState, funcname);
	if(lua_pcall(m_pState, 1, 0, 0)!=0) {
		SetErrorMsg(-1);
		return false;
	}

	return true;
}

bool CScriptState::Call(const char* funcname, void* obj, const char* obj_type, unsigned int cmd, unsigned int who, void* data, const char* data_type)
{
	assert(m_pState);

	return true;
}

void CScriptState::SetErrorMsg(int index)
{
	const char* msg;
	unsigned int len;
	len = lua_strlen(m_pState, index);
	msg = lua_tostring(m_pState, index);
	if(len<sizeof(m_szErrorMsg)) {
		memcpy(m_szErrorMsg, msg, len+1);
	} else {
		memcpy(m_szErrorMsg, msg, sizeof(m_szErrorMsg)-1);
		m_szErrorMsg[sizeof(m_szErrorMsg)-1] = '\0';
	}
}

IScriptState* CreateScriptState()
{
	return new CScriptState;
}
