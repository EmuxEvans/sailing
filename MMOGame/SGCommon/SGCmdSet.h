#pragma once

class CSGCmdSetManage
{
public:
	CSGCmdSetManage();

	CCmdSet& GetClientCmdSet() { return m_ClientCmdSet; }
	CCmdSet& GetServerCmdSet() { return m_ServerCmdSet; }

private:
	CCmdSet m_ClientCmdSet;
	CCmdSet m_ServerCmdSet;
};
