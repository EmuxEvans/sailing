#include <stddef.h>
#include <assert.h>
#include "Delegate.h"

IDelegate0::IDelegate0()
{
	m_pDelegate = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;
}

IDelegate0::~IDelegate0()
{
	if(m_pDelegate) {
		m_pDelegate->Unregister(this);
	}
}
