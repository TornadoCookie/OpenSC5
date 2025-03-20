
#include "rlWebkitThreading.h"
#include <sstream>

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
#include <windows.h> // Sleep
#elif defined(GLWEBKIT_PLATFORM_LINUX)
#include <unistd.h> // usleep
#define Sleep(x) (usleep(x*1000000))
#endif

StdMutex::StdMutex()
{
}

StdMutex::~StdMutex()
{

}

void StdMutex::Lock()
{
   m_mutex.lock();
}

bool StdMutex::TryLock() 
{
   return m_mutex.try_lock();
}

void StdMutex::Unlock() 
{
   m_mutex.unlock();
}

StdThreadCondition::StdThreadCondition() 
{
}

void StdThreadCondition::Wait(EA::WebKit::IMutex* _mutex) 
{
   StdMutex* mutex = dynamic_cast<StdMutex*>(_mutex);
   if(!mutex) 
      return;

   std::unique_lock<std::mutex> lock(mutex->m_mutex, std::defer_lock);
   m_cond_var.wait(lock);
}

bool StdThreadCondition::TimedWait(EA::WebKit::IMutex* _mutex, double relativeTimeMS)
{
   StdMutex* mutex = dynamic_cast<StdMutex*>(_mutex);
   if(!mutex) 
      return false;

   std::unique_lock<std::mutex> lock(mutex->m_mutex, std::defer_lock);

   std::cv_status status = m_cond_var.wait_for(lock, std::chrono::milliseconds((long)relativeTimeMS));
   return status == std::cv_status::no_timeout; // what return means???
}

void StdThreadCondition::Signal(bool broadcast) 
{
   if(broadcast) 
   {
      m_cond_var.notify_all();
   }
   else 
   {
      m_cond_var.notify_one();
   }
}

EA::WebKit::ThreadId StdThread::Begin(EA::WebKit::ThreadFunc fn, void* pThreadContext, void* pUserData)
{
   m_thread = std::thread(fn, pThreadContext);
   return (EA::WebKit::ThreadId)m_thread.native_handle();
}

void StdThread::WaitForEnd(intptr_t* result)
{
   m_thread.join();
}

void StdThread::SetName(const char* pName)
{
   //TODO
}


void StdThread::ChangePriority(int delta)
{
   //TODO
}

StdThreadLocalStorage::StdThreadLocalStorage() : m_value(0)
{
   //TODO
}
/*
void* Win64ThreadLocalStorage::GetValue()
{
    return m_value;
}

bool Win64ThreadLocalStorage::SetValue(void* pData)
{
    m_value = pData;
    return false;
}
*/


bool StdThreadSystem::Initialize()
{
   main_thread_id = std::this_thread::get_id();
   return true;
}

bool StdThreadSystem::Shutdown()
{
   return true;
}

EA::WebKit::IMutex* StdThreadSystem::CreateAMutex()
{
   return new StdMutex();
}

void StdThreadSystem::DestroyAMutex(EA::WebKit::IMutex* _mutex)
{
   StdMutex* mutex = (StdMutex*)_mutex;
   delete mutex;
}

EA::WebKit::IThreadCondition* StdThreadSystem::CreateAThreadCondition()
{
   return new StdThreadCondition();
}

void StdThreadSystem::DestroyAThreadCondition(EA::WebKit::IThreadCondition* _threadCondition)
{
   StdThreadCondition* threadCondition = (StdThreadCondition*)_threadCondition;
   delete threadCondition;
}

EA::WebKit::IThreadLocalStorage* StdThreadSystem::CreateAThreadLocalStorage()
{
   return new StdThreadLocalStorage();
}

void StdThreadSystem::DestroyAThreadLocalStorage(EA::WebKit::IThreadLocalStorage* _threadLocalStorage)
{
   StdThreadLocalStorage* threadLocalStorage = (StdThreadLocalStorage*)_threadLocalStorage;
   delete threadLocalStorage;
}

EA::WebKit::IThread* StdThreadSystem::CreateAThread()
{
   return new StdThread();
}

void StdThreadSystem::DestroyAThread(EA::WebKit::IThread* _pThread)
{
   StdThread* pThread = (StdThread*)_pThread;
   delete pThread;
}

//void StdThreadSystem::ScheduleWork(EA::WebKit::ThreadFunc doWork, void* workContext)
//{
//   doWork(workContext);
//}
EA::WebKit::ThreadId StdThreadSystem::CurrentThreadId()
{
   std::stringstream ss;
   ss << std::this_thread::get_id();
   return std::stoul(ss.str());
}

bool StdThreadSystem::IsMainThread()
{
   return (std::this_thread::get_id() == main_thread_id);
}

void StdThreadSystem::YieldThread()
{
}

void StdThreadSystem::SleepThread(uint32_t ms)
{
   std::this_thread::sleep_for(std::chrono::milliseconds((long)ms));
}
