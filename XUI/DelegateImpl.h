
#define MY_COMBINE(a,b)				MY_COMBINE1(a,b)
#define MY_COMBINE1(a,b)				a##b

MY_TEMPLATE MY_TEMPLATE_PARAMS
class MY_COMBINE(IDelegate, MY_SUFFIX);
MY_TEMPLATE MY_TEMPLATE_PARAMS
class MY_COMBINE(Delegate, MY_SUFFIX);

MY_TEMPLATE MY_TEMPLATE_PARAMS
class MY_COMBINE(IDelegate, MY_SUFFIX)
{
	friend class MY_COMBINE(Delegate, MY_SUFFIX) MY_TEMPLATE_ARGS;
public:
	MY_COMBINE(IDelegate, MY_SUFFIX) ();
	~MY_COMBINE(IDelegate, MY_SUFFIX) ();

	virtual void Invoke(MY_PARAMS) = 0;

protected:
	MY_COMBINE(Delegate, MY_SUFFIX)	MY_TEMPLATE_ARGS * m_pDelegate;
	MY_COMBINE(IDelegate, MY_SUFFIX)	MY_TEMPLATE_ARGS * m_pNext;
	MY_COMBINE(IDelegate, MY_SUFFIX)	MY_TEMPLATE_ARGS * m_pPrev;
};

MY_TEMPLATE MY_TEMPLATE_PARAMS
class MY_COMBINE(Delegate, MY_SUFFIX) {
public:
	MY_COMBINE(Delegate, MY_SUFFIX)() {
		m_pFirst = NULL;
	}
	~MY_COMBINE(Delegate, MY_SUFFIX)() {
		while(m_pFirst) {
			Unregister(m_pFirst);
		}
	}

	void Invoke(MY_PARAMS) {
		if(m_pFirst) {
			m_pFirst->Invoke(MY_ARGS);
		}
	}

	void Register(MY_COMBINE(IDelegate, MY_SUFFIX) MY_TEMPLATE_ARGS* pCall) {
		assert(pCall->m_pDelegate==NULL);
		pCall->m_pDelegate = this;
		pCall->m_pPrev = NULL;
		pCall->m_pNext = m_pFirst;
		m_pFirst = pCall;
	}

	void Unregister(MY_COMBINE(IDelegate, MY_SUFFIX) MY_TEMPLATE_ARGS* pCall) {
		assert(pCall->m_pDelegate==this);
		if(pCall->m_pNext) pCall->m_pNext->m_pPrev = pCall->m_pPrev;
		if(pCall->m_pPrev) {
			pCall->m_pPrev->m_pNext = pCall->m_pNext;
		} else {
			m_pFirst = pCall->m_pNext;
		}
		pCall->m_pDelegate = NULL;
		pCall->m_pPrev = NULL;
		pCall->m_pNext = NULL;
	}

	MY_COMBINE(IDelegate, MY_SUFFIX) MY_TEMPLATE_ARGS * m_pFirst;
};

#if MY_SUFFIX>0
template MY_TEMPLATE_PARAMS
MY_COMBINE(IDelegate, MY_SUFFIX) MY_TEMPLATE_ARGS ::MY_COMBINE(IDelegate, MY_SUFFIX) () {
	m_pDelegate = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;
}

template MY_TEMPLATE_PARAMS
MY_COMBINE(IDelegate, MY_SUFFIX) MY_TEMPLATE_ARGS ::~MY_COMBINE(IDelegate, MY_SUFFIX) () {
	if(m_pDelegate) {
		m_pDelegate->Unregister(this);
	}
}
#endif

template MY_T_TEMPLATE_PARAMS
class MY_COMBINE(DelegateImpl, MY_SUFFIX) : private MY_COMBINE(IDelegate, MY_SUFFIX) MY_TEMPLATE_ARGS {
public:
	MY_COMBINE(DelegateImpl, MY_SUFFIX) () {
		m_pThis = NULL;
		m_pInvoke = NULL;
	}
	~MY_COMBINE(DelegateImpl, MY_SUFFIX) () {
	}


	MY_COMBINE(IDelegate, MY_SUFFIX) MY_TEMPLATE_ARGS* R(T* pThis, void (T::*pInvoke)(MY_PARAMS)) {
		m_pThis = pThis;
		m_pInvoke = pInvoke;
		return this;
	}


private:
	virtual void Invoke(MY_PARAMS) {
		(m_pThis->*m_pInvoke)(MY_ARGS);
	}

	T* m_pThis;
	void (T::*m_pInvoke)(MY_PARAMS);
};

#undef MY_SUFFIX
#undef MY_TEMPLATE
#undef MY_TEMPLATE_PARAMS
#undef MY_TEMPLATE_ARGS
#undef MY_PARAMS
#undef MY_ARGS
#undef MY_T_TEMPLATE_PARAMS
