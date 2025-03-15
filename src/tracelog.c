#include "tracelog.h"
#include <cpl_raylib.h>
#include <stdlib.h>

static int logTypeLevel = LOG_INFO;

// Raylib's TraceLog, adapted to use va_list and whether or not to use a newline

#define MAX_TRACELOG_MSG_LENGTH     256         // Max length of one trace-log message
// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
// +OpenSC5Change 01/29/2025
void vTraceLog(int logType, const char *text, va_list args, bool newline)
// -OpenSC5Change
{
    // Message has level below current threshold, don't emit
    if (logType < logTypeLevel) return;

    //if (traceLog)
    //{
    //    traceLog(logType, text, args);
    //    va_end(args);
    //    return;
    //}

#if defined(PLATFORM_ANDROID)
    switch (logType)
    {
        case LOG_TRACE: __android_log_vprint(ANDROID_LOG_VERBOSE, "raylib", text, args); break;
        case LOG_DEBUG: __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", text, args); break;
        case LOG_INFO: __android_log_vprint(ANDROID_LOG_INFO, "raylib", text, args); break;
        case LOG_WARNING: __android_log_vprint(ANDROID_LOG_WARN, "raylib", text, args); break;
        case LOG_ERROR: __android_log_vprint(ANDROID_LOG_ERROR, "raylib", text, args); break;
        case LOG_FATAL: __android_log_vprint(ANDROID_LOG_FATAL, "raylib", text, args); break;
        default: break;
    }
#else
    char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };

    switch (logType)
    {
        case LOG_TRACE: strcpy(buffer, "TRACE: "); break;
        case LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
        case LOG_INFO: strcpy(buffer, "INFO: "); break;
        case LOG_WARNING: strcpy(buffer, "WARNING: "); break;
        case LOG_ERROR: strcpy(buffer, "ERROR: "); break;
        case LOG_FATAL: strcpy(buffer, "FATAL: "); break;
        default: break;
    }

    unsigned int textSize = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textSize < (MAX_TRACELOG_MSG_LENGTH - 12))? textSize : (MAX_TRACELOG_MSG_LENGTH - 12));
// +OpenSC5Change 01/29/2025
    if (newline) strcat(buffer, "\n");
// -OpenSC5Change
    vprintf(buffer, args);
    fflush(stdout);
#endif

    if (logType == LOG_FATAL) exit(EXIT_FAILURE);  // If fatal logging, exit program
}

static void tl_prep(void)
{
#    printf("[%06x]:", gettid());
}

static void tl_end(void)
{
    
}

void OpenSC5_TraceLog(int logLevel, const char *text, ...)
{
    // originally we used printf, so every log message ended with a newline
    // TraceLog by default ends every line with a newline, so we need to remove it.

    char *ourText = strdup(text);

    if (ourText[strlen(text) - 1] == '\n')
    {
        ourText[strlen(text) - 1] = 0;
    }

    if (logLevel < logTypeLevel) return; 

    tl_prep();

    va_list args;
    va_start(args, text);

    vTraceLog(logLevel, ourText, args, true);

    va_end(args);

    tl_end();
}

void OpenSC5_TraceLogNoNL(int logLevel, const char *text, ...)
{
    // this function is like the previous one but it behaves like printf, where there is no newline at the end.

    if (logLevel < logTypeLevel) return;

    va_list args;
    va_start(args, text);

    vprintf(text, args);

    va_end(args);
}

void OpenSC5_SetTraceLogLevel(int logLevel)
{
    logTypeLevel = logLevel;
}

