#ifndef _TRACELOG_
#define _TRACELOG_

#ifdef __cplusplus
extern "C" {
#endif

void OpenSC5_TraceLog(int logLevel, const char *text, ...);
void OpenSC5_TraceLogNoNL(int logLevel, const char *text, ...);
void OpenSC5_SetTraceLogLevel(int logLevel);

#ifdef __cplusplus
}
#endif

#ifndef TRACELOG

// we will add debug channels later. for now this is good.
//#define OPENSC5_DEBUG_CHANNEL(ch) static const char *__osc5_dbg_channel = (ch);

#define TRACELOG OpenSC5_TraceLog
#define TRACELOGNONL OpenSC5_TraceLogNoNL

#endif

#endif

