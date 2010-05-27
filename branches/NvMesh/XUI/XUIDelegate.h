#pragma once

#include "sigslot.h"

class XUIWidget;

#define XUI_DELEGATE_EMPTY()
#define XUI_DELEGATE_COMBINE(a,b)				XUI_DELEGATE_COMBINE1(a,b)
#define XUI_DELEGATE_COMBINE1(a,b)				a##b

#define XUI_DELEGATE_DEFINE0(name)														\
typedef sigslot::signal1<XUIWidget*> name;

#define XUI_DELEGATE_DEFINE1(name, ...)													\
typedef sigslot::signal2<XUIWidget*, __VA_ARGS__> name;

#define XUI_DELEGATE_DEFINE2(name, ...)													\
typedef sigslot::signal3<XUIWidget*, __VA_ARGS__> name;

#define XUI_DELEGATE_DEFINE3(name, ...)													\
typedef sigslot::signal4<XUIWidget*, __VA_ARGS__> name;

#define XUI_DELEGATE_DEFINE4(name, ...)													\
typedef sigslot::signal5<XUIWidget*, __VA_ARGS__> name;

#define XUI_DELEGATE_DEFINE5(name, ...)													\
typedef sigslot::signal6<XUIWidget*, __VA_ARGS__> name;
