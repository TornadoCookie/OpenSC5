
#pragma once

#include "EAWebKit/EAWebKitThreadInterface.h"

#include <thread>
#include <mutex>
#include <condition_variable>

class StdMutex : public EA::WebKit::IMutex
{
public:
   friend class DefaultThreadCondition;
   friend class StdThreadCondition;
   StdMutex();
   virtual ~StdMutex();
   void Lock();
   bool TryLock();
   void Unlock();

private:
   std::mutex m_mutex;
};

class StdThreadCondition : public EA::WebKit::IThreadCondition
{
public:
   StdThreadCondition();
   void Wait(EA::WebKit::IMutex* mutex);
   bool TimedWait(EA::WebKit::IMutex* mutex, double relativeTimeMS);
   void Signal(bool broadcast);
private:
   std::condition_variable m_cond_var;
};


class StdThread : public EA::WebKit::IThread
{
public:
   virtual EA::WebKit::ThreadId Begin(EA::WebKit::ThreadFunc, void* pThreadContext, void* pUserData);
   virtual void WaitForEnd(intptr_t* result);
   virtual void SetName(const char* pName);
   virtual void ChangePriority(int delta);
protected:
   std::thread m_thread;
};

class StdThreadLocalStorage : public EA::WebKit::IThreadLocalStorage
{
public:
   StdThreadLocalStorage();
   virtual void* GetValue() { return m_value; }
   bool SetValue(void* pData) { m_value = pData; return true; }
private:
   void* m_value;
};

class StdThreadSystem : public EA::WebKit::IThreadSystem
{
public:
   virtual bool Initialize();
   virtual bool Shutdown();

   virtual EA::WebKit::IMutex* CreateAMutex();
   virtual void DestroyAMutex(EA::WebKit::IMutex*);

   virtual EA::WebKit::IThreadCondition* CreateAThreadCondition();
   virtual void DestroyAThreadCondition(EA::WebKit::IThreadCondition*);

   virtual EA::WebKit::IThreadLocalStorage* CreateAThreadLocalStorage();
   virtual void DestroyAThreadLocalStorage(EA::WebKit::IThreadLocalStorage*);

   virtual EA::WebKit::IThread* CreateAThread();
   virtual void DestroyAThread(EA::WebKit::IThread*);

   virtual void ScheduleWork(EA::WebKit::ThreadFunc doWork, void* workContext);
   virtual EA::WebKit::ThreadId CurrentThreadId();
   virtual bool IsMainThread();
   virtual void YieldThread();
   virtual void SleepThread(uint32_t ms);
protected:
   std::thread::id main_thread_id;
};
