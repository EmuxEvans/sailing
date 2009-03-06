#pragma once

#ifndef _WIN32

typedef unsigned char		os_byte;
typedef unsigned short		os_word;
typedef unsigned int		os_dword;
typedef unsigned long long	os_qword;
typedef char				os_char;
typedef short				os_short;
typedef int					os_int;
typedef long long			os_long;
typedef float				os_float;
typedef double				os_double;

#else

typedef unsigned char		os_byte;
typedef unsigned short		os_word;
typedef unsigned int		os_dword;
typedef unsigned __int64	os_qword;
typedef char				os_char;
typedef short				os_short;
typedef int					os_int;
typedef __int64				os_long;
typedef float				os_float;
typedef double				os_double;

#endif

#include "Math.h"
#include "CmdData.h"

#include "GameLoop.h"
#include "GameArea.h"
#include "GameArea.inl"
#include "GameBuff.h"
#include "GameItem.h"
#include "GameSkill.h"
#include "GameQuest.h"
