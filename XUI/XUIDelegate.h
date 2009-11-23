#pragma once

#include "Delegate.h"

class XUIWidget;

#define XUI_DELEGATE_EMPTY()
#define XUI_DELEGATE_COMBINE(a,b)				XUI_DELEGATE_COMBINE1(a,b)
#define XUI_DELEGATE_COMBINE1(a,b)				a##b

#define XUI_DELEGATE_DEFINE0(name)														\
typedef Delegate1<XUIWidget*> name;														\
template<class T>																		\
class XUI_DELEGATE_COMBINE(XUI_DELEGATE_EMPTY()##name, Impl) : public DelegateImpl1<T, XUIWidget*> {};					\

#define XUI_DELEGATE_DEFINE1(name, ...)													\
typedef Delegate2<XUIWidget*, __VA_ARGS__> name;										\
template<class T>																		\
class XUI_DELEGATE_COMBINE(XUI_DELEGATE_EMPTY()##name, Impl) : public DelegateImpl2<T, XUIWidget*, __VA_ARGS__> {};	\

#define XUI_DELEGATE_DEFINE2(name, ...)													\
typedef Delegate3<XUIWidget*, __VA_ARGS__> name;										\
template<class T>																		\
class XUI_DELEGATE_COMBINE(XUI_DELEGATE_EMPTY()##name, Impl) : public DelegateImpl3<T, XUIWidget*, __VA_ARGS__> {};	\

#define XUI_DELEGATE_DEFINE3(name, ...)													\
typedef Delegate4<XUIWidget*, __VA_ARGS__> name;										\
template<class T>																		\
class XUI_DELEGATE_COMBINE(XUI_DELEGATE_EMPTY()##name, Impl) : public DelegateImpl4<T, XUIWidget*, __VA_ARGS__> {};	\

#define XUI_DELEGATE_DEFINE4(name, ...)													\
typedef Delegate5<XUIWidget*, __VA_ARGS__> name;										\
template<class T>																		\
class XUI_DELEGATE_COMBINE(XUI_DELEGATE_EMPTY()##name, Impl) : public DelegateImpl5<T, XUIWidget*, __VA_ARGS__> {};	\

/*
#undef XUI_DELEGATE_EMPTY
#undef XUI_DELEGATE_COMBINE
#undef XUI_DELEGATE_COMBINE1
*/