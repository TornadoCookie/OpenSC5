#ifndef _THREADPOOL_
#define _THREADPOOL_

void InitThreadpool(int nproc);
void NewThreadpoolTask(void (*task)(void*), void *arg);
void WaitForThreadpoolTasksDone(void);
int GetThreadpoolTasksLeft(void);
void CloseThreadpool(void);

#endif
