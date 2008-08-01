#ifndef __ZIONOS_INCLUDE_
#define __ZIONOS_INCLUDE_

#ifdef __LINUX__
#	include "os_unix.h"
#endif

#ifdef __FREEBSD__
#	include "os_unix.h"
#endif

#ifdef __MACOSX__
#	include "os_unix.h"
#endif

#ifdef _WIN32
#	define _CRTDBG_MAP_ALLOC
#	include <crtdbg.h>
#	include "os_win32.h"
#endif

#endif
