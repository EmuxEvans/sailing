#pragma once

typedef struct CmdData {
	unsigned int nCmd;
	unsigned int nWho;
	const void* pData;
	unsigned int nSize;
} CmdData;

class CCmdDataReader {
public:
	CCmdDataReader(const CmdData* pCmdData) {
		m_pCmdData = pCmdData;
		Reset();
	}

	void Reset() {
		m_nCurrent = 0;
	}

	unsigned int CmdCode() {
		return m_pCmdData->nCmd;
	}
	unsigned int CmdWho() {
		return m_pCmdData->nWho;
	}

	bool GetFloat(float& fValue, float fMin=0.0f, float fMax=0.0f) {
		if(m_nCurrent+sizeof(float)>m_pCmdData->nSize) return false;
		fValue = *((const float*)((const char*)m_pCmdData->pData + m_nCurrent));
		if((fMin!=0 || fMax!=0) && (fValue<fMin || fValue>fMax)) return false;
		m_nCurrent += sizeof(float);
		return true;		
	}
	bool GetInteger(int& nValue, int nMin=0, int nMax=0) {
		if(m_nCurrent+sizeof(int)>m_pCmdData->nSize) return false;
		nValue = *((const int*)((const char*)m_pCmdData->pData + m_nCurrent));
		if((nMin!=0 || nMax!=0) && (nValue<nMin || nValue>nMax)) return false;
		m_nCurrent += sizeof(int);
		return true;		
	}
	bool GetString(const char*& pValue, unsigned int nMaxLen) {
		unsigned int n;
		for(n=0; n<=nMaxLen && m_nCurrent+n<m_pCmdData->nSize; n++) {
			if(*((const char*)m_pCmdData->pData + m_nCurrent)=='\0') {
				pValue = (const char*)m_pCmdData->pData + m_nCurrent;
				m_nCurrent += n;
				return true;
			}
		}
		return false;
	}

private:
	const CmdData* m_pCmdData;
	unsigned int m_nCurrent;
};

class CCmdDataWriter {
public:
	CCmdDataWriter(CmdData* pCmdData) {
		m_pCmdData = pCmdData;
		Reset();
	}

	void Reset() {
		m_nCurrent = 0;
		m_nSize = m_pCmdData->nSize;
	}

	void CmdCode(unsigned int nCmd) {
		m_pCmdData->nCmd = nCmd;
	}
	void CmdWho(unsigned int nWho) {
		m_pCmdData->nWho = nWho;
	}

	bool PutFloat(float fValue) {
		if(m_nCurrent+sizeof(float)>m_nSize) return false;
		*((float*)((char*)m_pCmdData->pData + m_nCurrent)) = fValue;
		m_nCurrent += sizeof(float);
		return true;		
	}
	bool PutInteger(int nValue) {
		if(m_nCurrent+sizeof(int)>m_nSize) return false;
		*((int*)((char*)m_pCmdData->pData + m_nCurrent)) = nValue;
		m_nCurrent += sizeof(int);
		return true;		
	}
	bool PutString(const char* pValue) {
		unsigned int n = 0;
		if(m_nCurrent+n+1<m_nSize) return false;
		memcpy((char*)m_pCmdData->pData+m_nCurrent, pValue, n+1);
		m_nCurrent += n + 1;
		return true;
	}

	unsigned int Size() {
		return m_nSize;
	}

private:
	CmdData* m_pCmdData;
	unsigned int m_nCurrent;
	unsigned int m_nSize;
};
