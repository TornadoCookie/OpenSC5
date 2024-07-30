#ifndef _CPL_PTHREAD_
#define _CPL_PTHREAD_

#ifdef _WIN32

#include <cpl_raylib.h>

typedef HANDLE pthread_t;

typedef struct _ThreadProcArgs {
    void *(*func)(void*);
    void *arg;
} _ThreadProcArgs;

static DWORD WINAPI _threadproc(void *param)
{
    _ThreadProcArgs *args = param;

    args->func(args->arg);
}

static void pthread_create(pthread_t *newThread, void *unused, void *(*func)(void *), void *arg)
{
    _ThreadProcArgs args = {func, arg};
    *newThread = CreateThread(NULL, 0, _threadproc, &args, 0, NULL);
}

static void pthread_cancel(pthread_t thread)
{
    TerminateThread(thread, 0);
    CloseHandle(thread);
}

typedef HANDLE pthread_mutex_t;

static void pthread_mutex_init(pthread_mutex_t *mutex, void *attr)
{
    *mutex = CreateMutex(NULL, FALSE, "opensc5_threadpool_worker");
    nextMutexId++;
}

static void pthread_mutex_lock(pthread_mutex_t *mutex)
{
    WaitForSingleObject(*mutex, INFINITE);
}

static void pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    ReleaseMutex(*mutex);
}

static void pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    CloseHandle(*mutex);
}

#else
#include <pthread.h>
#endif

#endif
