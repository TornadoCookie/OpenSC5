#include "threadpool.h"
#include <pthread.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <raylib.h>
#include <stdio.h>

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
    int nproc_true = get_nprocs();
    if (nproc == -1)
    {
        nproc = nproc_true;
    }

    if (nproc > nproc_true)
    {
        printf("decreasing actual threads used from %d to %d.\n", nproc, nproc_true);
        nproc = nproc_true;
    }

    printf("Initialized threadpool with %d threads.\n", nproc);

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
            printf("%d tasks left...\n", taskCount);
            prevTaskCount = taskCount;
        }
        WaitTime(1.0f/60);
    }
    printf("done.\n");
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
