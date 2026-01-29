#include "rlWebKit/rlWebKit.h"

#include "rlWebkitUtils.h"
#include "rlWebkitRenderer.h"
#include "rlWebkitThreading.h"
#include "rlWebkitClient.h"

#include <EABase/eabase.h>
#include <EAWebKit/EAWebKit>
#include <EAWebKit/EAWebKitDll.h>
#include <EAText/EAText.h>

#include "DBPFFileSystem.hpp"

//#include <DirtySDK/dirtysock/netconn.h>

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
#include <windows.h>
#include <bcrypt.h>
#elif defined(GLWEBKIT_PLATFORM_LINUX)
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#endif

#include <list>
#include <iostream>
#include <raylib.h>

EA::WebKit::EAWebKitLib* wk = nullptr;

typedef EA::WebKit::EAWebKitLib* (*PF_CreateEAWebkitInstance)(void);

#if defined(GLWEBKIT_PLATFORM_WINDOWS)

// Callbacks
double timerCallback() 
{ 
    LARGE_INTEGER frequency;
    ::QueryPerformanceFrequency(&frequency);

LARGE_INTEGER start;
    ::QueryPerformanceCounter(&start);

    return static_cast<double>(start.QuadPart) / (double)frequency.QuadPart;
}

double monotonicTimerCallback() 
{
    return timerCallback();
};

void cryptographicallyRandomValueCallback(unsigned char *buffer, size_t length)
{
   //deprecated API, using newer api below
//     HCRYPTPROV hCryptProv = 0;
//     CryptAcquireContext(&hCryptProv, 0, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
//     CryptGenRandom(hCryptProv, length, buffer);
//     CryptReleaseContext(hCryptProv, 0);

    BCRYPT_ALG_HANDLE algorithm;
    NTSTATUS ret;
    ret = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RNG_ALGORITHM, 0, 0);
    if(ret != 0)
    {
       std::cout << "Failed to initialize cryto algorithm provider.  Return code: " << ret << std::endl;
       return;
    }

    ret = BCryptGenRandom(algorithm, buffer, length, 0);
    if(ret != 0)
    {
       std::cout << "Failed to get cryto random number.  Return code: " << ret << std::endl;
       return;
    }
    
    //return true;  // Returns true if no error, else false
}

void* stackBaseCallback() 
{
   //taken from: https://github.com/adobe/webkit/blob/master/Source/WTF/wtf/StackBounds.cpp#L228
   PNT_TIB64 pTib = reinterpret_cast<PNT_TIB64>(NtCurrentTeb());
   return reinterpret_cast<void*>(pTib->StackBase);
}

PF_CreateEAWebkitInstance get_CreateEAWebKitInstance()
{
#ifdef _DEBUG
   HMODULE wdll = LoadLibraryA("EAWebkitd.dll");
#else
   HMODULE wdll = LoadLibraryA("EAWebkit.dll");
#endif // _DEBUG
if(wdll != nullptr)
   {
      return reinterpret_cast<PF_CreateEAWebkitInstance>(GetProcAddress(wdll, "CreateEAWebkitInstance"));
   }

   return nullptr;
}

#elif defined(GLWEBKIT_PLATFORM_LINUX)

// Callbacks
double timerCallback()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    double t = (double)ts.tv_sec + ((double)ts.tv_nsec) / 1000000000;

    return t;
}

double monotonicTimerCallback()
{
    return timerCallback();
}

void cryptographicallyRandomValueCallback(unsigned char *buffer, size_t length)
{
    // use /dev/random because it is more secure
    FILE *f = fopen("/dev/random", "rb");

    if (!f) return;

    if (fread(buffer, 1, length, f) != length) return;

    fclose(f);

    //return true;
}

#include <pthread.h>

void *stackBaseCallback()
{
    void* stackBase = 0;
    size_t stackSize = 0;

    pthread_t thread = pthread_self();
    pthread_attr_t sattr;
    pthread_attr_init(&sattr);
//#if HAVE(PTHREAD_NP_H) || OS(NETBSD)
    // e.g. on FreeBSD 5.4, neundorf@kde.org
//    pthread_attr_get_np(thread, &sattr);
//#else
    // FIXME: this function is non-portable; other POSIX systems may have different np alternatives
    pthread_getattr_np(thread, &sattr);
//#endif
    int rc = pthread_attr_getstack(&sattr, &stackBase, &stackSize);
    (void)rc; // FIXME: Deal with error code somehow? Seems fatal.
    //ASSERT(stackBase);
    pthread_attr_destroy(&sattr);
    //m_bound = stackBase;
    return static_cast<char*>(stackBase) + stackSize; 
}

static EA::WebKit::EAWebKitLib *linux_CreateEAWebkitInstance()
{
    static EA::WebKit::EAWebKitLib concreteInstance;
    return &concreteInstance;
}

PF_CreateEAWebkitInstance get_CreateEAWebKitInstance()
{
    return linux_CreateEAWebkitInstance;
}

#endif

//void getCookiesCallback(const char16_t* pUrl, EA::WebKit::EASTLFixedString16Wrapper& result, uint32_t flags)
//{
//   std::cout << __FUNCTION__ << std::endl;
//}

//bool setCookieCallback(const EA::WebKit::CookieEx& cookie)
//{
//   std::cout << __FUNCTION__ << std::endl;
//   return false;  
//}

struct EA::WebKit::AppCallbacks callbacks = {
   timerCallback,
   stackBaseCallback,
   NULL, //atomicIncrementCallback
   NULL, //atomicDecrementCallback
   cryptographicallyRandomValueCallback, //cryptRandomValueCallback
};

static GLWebkitClient wkC;
static StdThreadSystem threadSystem;
static DBPFFileSystem fileSystem;

// init the systems: using DefaultAllocator, DefaultFileSystem, no text/font support, DefaultThreadSystem
struct EA::WebKit::AppSystems systems = {
    NULL,// mAllocator
    &fileSystem,// mFileSystem
    NULL,// mTextSystem
    &threadSystem,// mThreadSystem
    &wkC,// mEAWebkitClient
};

static char16_t *tochar16(const char *str)
{
    char16_t *ret = new char16_t[strlen(str)+1];

    for (int i = 0; i < strlen(str)+1; i++)
    {
        ret[i] = str[i];
    }

    return ret;
}

bool initWebkit()
{
   PF_CreateEAWebkitInstance create_Webkit_instance = get_CreateEAWebKitInstance();

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
   // init winsock manually, this is required
   WSADATA wsadata = {};
   WSAStartup(MAKEWORD(2, 0), &wsadata);

#endif

   if (!create_Webkit_instance)
   {
      std::cout << "EAWebkit.dll missing" << std::endl;
      return false;
   }

   wk = create_Webkit_instance();

   //check that dll is same version as our headers
   const char* verStr = wk->GetVersion();
   if (strcmp(verStr, EAWEBKIT_VERSION_S) != 0)
   {
      std::cout << "Error!  Mismatched versions of EA Webkit" << std::endl;
      return false;
   }
   
   //initialize the system
   wk->Init(&callbacks, &systems);

   EA::WebKit::Parameters& params = wk->GetParameters();
   //params.mEAWebkitLogLevel = 4;
   //params.mHttpManagerLogLevel = 4;
   //params.mRemoteWebInspectorPort = 1234;
   params.mReportJSExceptionCallstacks = true;
   params.mVerifySSLCert = true;
   params.mJavaScriptStackSize = 1024 * 1024; //1MB of stack space
   params.mJavaScriptDebugOutputEnabled = true;
   
   wk->SetParameters(params);

   wk->AddTransportHandler(wk->GetTransportHandler(tochar16("file")), tochar16(""));

   //NetConnStartup("-servicename=rlWebKit");

   EA::Text::Init();

   //should be pulling these from the OS by their family type
   //times new roman is the default fallback if a font isn't found, so we need 
   //to at least load this (should probably be built in)
   int ret = add_ttf_font(wk, "times.ttf");
   if (ret == 0)
   {
      std::cout << "Error adding times.ttf font. " << std::endl;
   }

   return true;
}

EA::WebKit::View* createView(int x, int y)
{
   EA::WebKit::View* v = 0;

   v = wk->CreateView();
   EA::WebKit::ViewParameters vp;
   vp.mHardwareRenderer = nullptr; // use default renderer
   vp.mDisplaySurface = nullptr; // use default surface
   vp.mWidth = x;
   vp.mHeight = y;
   vp.mBackgroundColor = 0; //clear  0xffffffff; //white
   vp.mTileSize = 256;
   vp.mUseTiledBackingStore = false;
   vp.mpUserData = v;
   v->InitView(vp);
   v->SetSize(EA::WebKit::IntSize(vp.mWidth, vp.mHeight));

   //v->ShowInspector(true);
   v->SetDrawDebugVisuals(true);

   return v;
}

bool shutdownWebKit()
{
   wk->Shutdown();

   return true;
}

void updateWebkit(EA::WebKit::View *v)
{
   if (!v) 
       return;

    v->Tick();
    
}

void setViewUrl(EA::WebKit::View* v, const char* url)
{
   v->SetURI(url);
}

void updateView(EA::WebKit::View* v)
{
    //v->Paint();
    v->ShowInspector(true);
}

void resize(EA::WebKit::View* v, int width, int height)
{
    if (!v) 
       return;

    v->SetSize(EA::WebKit::IntSize(width, height));
}

void mousemove(EA::WebKit::View* v, int x, int y)
{
    if (!v) 
       return;

    EA::WebKit::MouseMoveEvent e = {};
    e.mX = x;
    e.mY = y;
    v->OnMouseMoveEvent(e);
}

void mousebutton(EA::WebKit::View* v, int x, int y, int btn, bool depressed)
{
    if (!v) 
       return;

    EA::WebKit::MouseButtonEvent e = {};
    e.mId = btn;
    e.mX = x;
    e.mY = y;
    e.mbDepressed = depressed;
    v->OnMouseButtonEvent(e);
}

void mousewheel(EA::WebKit::View* v, int x, int y, int keys, int delta)
{
    if (!v) 
       return;

    EA::WebKit::MouseWheelEvent e = {};
    e.mX = x;
    e.mY = y;
    e.mZDelta = delta;

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
    UINT scrollLines = 1;
    SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);
    e.mNumLines = ((delta * (int32_t)scrollLines) / (int32_t)WHEEL_DELTA);
#elif defined(GLWEBKIT_PLATFORM_LINUX)
    e.mNumLines = delta;
#endif

    v->OnMouseWheelEvent(e);
}

void keyboard(EA::WebKit::View* v, int id, bool ischar, bool depressed)
{
    if (!v) 
       return;

    EA::WebKit::KeyboardEvent e = {};
    e.mId = id;
    e.mbChar = ischar;
    e.mbDepressed = depressed;
    v->OnKeyboardEvent(e);
}

void reload(EA::WebKit::View* v)
{
    if (!v)
       return;

    v->Refresh();
}

void destroyView(EA::WebKit::View* v)
{
   if(!wk || !v)
      return;

   wk->DestroyView(v);
}
