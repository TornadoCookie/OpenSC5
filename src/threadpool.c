#include "threadpool.h"
#include <cpl_pthread.h>
#include <stdlib.h>
#include <cpl_raylib.h>
#include <stdio.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#elif defined(_WIN32)
#include <sysinfoapi.h>
#endif

static pthread_t *threadpool;
static int threadpoolCount;

typedef struct ThreadpoolTask {
    void (*task)(void*);
    void *arg;
} ThreadpoolTask;

static ThreadpoolTask *tasks;
static int taskCount;
static pthread_mutex_t task_mutex;

void *threadpool_runner(void *__unused_arg)
{
    while (1)
    {
        if (!taskCount)
        {
            WaitTime(1.0f/60);
            continue;
        }
        pthread_mutex_lock(&task_mutex);
        ThreadpoolTask task = tasks[taskCount - 1];
        taskCount--;
        tasks = realloc(tasks, taskCount * sizeof(ThreadpoolTask));
        pthread_mutex_unlock(&task_mutex);

        task.task(task.arg);
    }
}

void InitThreadpool(int nproc)
{
    int nproc_true = 1;

    #ifdef __linux__
    nproc_true = get_nprocs();
    #elif defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    nproc_true = sysInfo.dwNumberOfProcessors;
    #else
    #warning Add a way to get number of processors to improve threadpool performance.
    #endif

    if (nproc == -1)
    {
        nproc = nproc_true;
    }

    if (nproc > nproc_true)
    {
        TRACELOG(LOG_WARNING, "decreasing actual threads used from %d to %d.", nproc, nproc_true);
        nproc = nproc_true;
    }

    TRACELOG(LOG_INFO, "Initialized threadpool with %d threads.", nproc);

    threadpoolCount = nproc;
    threadpool = malloc(sizeof(pthread_t) * threadpoolCount);

    pthread_mutex_init(&task_mutex, NULL);

    for (int i = 0; i < threadpoolCount; i++)
    {
        pthread_create(&threadpool[i], NULL, threadpool_runner, NULL);
    }
}

void NewThreadpoolTask(void (*task)(void*), void *arg)
{
    pthread_mutex_lock(&task_mutex);
    taskCount++;
    tasks = realloc(tasks, sizeof(ThreadpoolTask) * taskCount);
    tasks[taskCount - 1] = (ThreadpoolTask){task, arg};
    pthread_mutex_unlock(&task_mutex);
}

void WaitForThreadpoolTasksDone(void)
{
    int prevTaskCount = 0;
    while (taskCount) {
        if (taskCount != prevTaskCount)
        {
            TRACELOG(LOG_INFO, "%d tasks left...", taskCount);
            prevTaskCount = taskCount;
        }
        WaitTime(1.0f/60);
    }
    TRACELOG(LOG_INFO, "done.");
}

int GetThreadpoolTasksLeft(void)
{
    return taskCount;
}

void CloseThreadpool(void)
{
    for (int i = 0; i < threadpoolCount; i++)
    {
        pthread_cancel(threadpool[i]);
    }
    free(tasks);
    free(threadpool);
    pthread_mutex_destroy(&task_mutex);
}
