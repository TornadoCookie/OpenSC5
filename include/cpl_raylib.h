#ifndef _CPL_RAYLIB_
#define _CPL_RAYLIB_

#ifdef _WIN32

#define _THREADPOOLAPISET_H_ // Disable threadpoolapiset.h as we have our own threadpool system that conflicts with it



#define NOGDI
#define NOUSER
#define MMNOSOUND

typedef struct tagMSG *LPMSG;

#include <windows.h>

#undef near
#undef far

#endif

#include "tracelog.h"
#include <raylib.h>
#include "textformat_ng.h"

#define SetTraceLogLevel OpenSC5_SetTraceLogLevel


#endif

