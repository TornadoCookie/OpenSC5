#ifndef _CPL_RAYLIB_
#define _CPL_RAYLIB_

#ifdef _WIN32

#define NOGDI
#define NOUSER
#define MMNOSOUND

typedef struct tagMSG *LPMSG;

#include <windows.h>

#undef near
#undef far

#endif

#include <raylib.h>
#include "textformat_ng.h"

#ifndef TRACELOG
#define TRACELOG TraceLog
#endif

#endif

