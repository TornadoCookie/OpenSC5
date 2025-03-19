/*
Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Electronic Arts, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
///////////////////////////////////////////////////////////////////////////////
// EAWebKit.cpp
// By Paul Pedriana 
// Maintained by EAWebKit Team
///////////////////////////////////////////////////////////////////////////////
 
#include "config.h"
#include <time.h>

#include <EAWebKit/EAWebKit.h>
#include <internal/include/EAWebKit_p.h>
#include <EAWebKit/EAWebkitSTLWrapper.h>
#include <internal/include/EAWebkitEASTLHelpers.h>
#include <internal/include/EAWebKitAssert.h>
#include <internal/include/EAWebKitJSTextWrapper.h>
#include <internal/include/EAWebKitDomainFilter.h>
#include <internal/include/EAWebKitFPUPrecision.h>
#include <internal/include/EAWebKitString.h>
#include <EAWebKit/EAWebKitClient.h>
#include <EAWebKit/EAWebKitView.h>
#include <EAWebKit/EAWebKitThreadInterface.h>
#include <EAIO/PathString.h>
#include <EAAssert/eaassert.h>

#include <API/JSSettingsEA.h>

#include "Logging.h"
#include "WebPlatformStrategies.h"
#include "ResourceHandleManagerEA.h"
#include "NetworkStateNotifier.h"
#include "MemoryCache.h"
#include "ea_class.h"
#include "RenderThemeEA.h"
#include "ScrollbarThemeEA.h"
#include "InspectorClientEA.h"
#include "SecurityOrigin.h"
#include "PageGroup.h"
#include "DOMWrapperWorld.h"


//+ Following are mainly included to fix leaks(call finalize() type functions).
#include "PageCache.h"
#include "QualifiedName.h"
#include "JSHTMLElementWrapperFactory.h"
#include "HTMLElementFactory.h"
#include "XMLNames.h"
#include "XMLNSNames.h"
#include "MathMLNames.h"
#include "XLinkNames.h"
#include "SVGNames.h"
#include "HTMLNames.h"
#include <wtf/Threading.h>
#include <wtf/CurrentTime.h>
#include <cairo.h>
#include <PageCache.h>
#include <FontCache.h>
#include <CrossOriginPreflightResultCache.h>
#include <CachedResource.h>
#include <GCController.h>
//-
#if ENABLE(EATEXT_IN_DLL)
#include <internal/include/EAWebKitTextWrapper.h>
#endif

#if ENABLE(DIRTYSDK_IN_DLL)
#include "protossl.h"
#include "platformsocketapi.h"
#include <EAWebKit/EAWebKitPlatformSocketAPI.h>

EA_COMPILETIME_ASSERT(sizeof(EA::WebKit::PlatformSocketAPI) == sizeof(PlatformSocketAPICallbacks));
EA_COMPILETIME_ASSERT((int)EA::WebKit::eSockErrCount == (int)eSockErrCount);
#endif

#if defined(EA_PLATFORM_MICROSOFT)
#include EAWEBKIT_PLATFORM_HEADER
#endif

// abaldeva: General note. 05/22/2012
// For the time being, we need to avoid calling wchar_t related functions in EAWebKit to keep it cross-platform.
// On Mac OS X, the build system adds -fshort-wchar option to compilation. This turns the size of wchar_t to be 2. 
// Now, the system libs are compiled with wchar_t to be 4 and we don't have compiled libs with wchar_t set at 2. 
// This results in SNAFU. If -fshort-wchar is removed (tried it locally), some other packages fail to compile. The solution is to avoid 
// calling the w* functions at the moment. 

namespace EA
{
namespace WebKit
{
const char8_t* kDefaultPageGroupName = "default"; // Any name would suffice here.

static WebKitStatus					sWebKitStatus = kWebKitStatusInactive;
static EAWebKitClient*				spEAWebKitClient = NULL;
static ITextSystem*                 spTextSystem = NULL;
static SocketTransportHandler*		spSocketTransportHandler = NULL;
static bool							ownsSocketTransportHandler = false;
static RAMCacheInfo					sRamCacheInfo;

// Some globals instead of statics as we need to use them in the JavaScriptCore project directly.
EAWebKitTimerCallback		        gpTimerCallback = NULL;
EAWebKitStackBaseCallback           gpStackBaseCallback = NULL;
void*								gpCollectorStackBase = NULL;
EAWebKitAtomicIncrementCallback     gpAtomicIncrement = NULL;
EAWebKitAtomicDecrementCallback     gpAtomicDecrement = NULL;
EAWebKitCryptographicallyRandomValueCallback gpCryptographicallyRandomValueCallback = NULL;

// We need to add appropriate asserts to each of these functions based on WebKit state.
// Consider WebKit assert system as the user installed EAWebKit assert system may not be present or go away too soon.

bool EAWebKitLib::Init(AppCallbacks* appCallbacks /* = NULL */, AppSystems* appSystems /* = NULL */)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);

	return EA::WebKit::Init(appCallbacks,appSystems);

}

void EAWebKitLib::Shutdown()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::Shutdown();
}

void EAWebKitLib::Destroy()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::Destroy();
}

Allocator* EAWebKitLib::GetAllocator()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetAllocator();
}


FileSystem* EAWebKitLib::GetFileSystem()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetFileSystem();
}
EAWebKitClient* EAWebKitLib::GetEAWebKitClient()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetEAWebKitClient();
}

EA::WebKit::ITextSystem* EAWebKitLib::GetTextSystem()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetTextSystem();
}
IThreadSystem* EAWebKitLib::GetThreadSystem()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetThreadSystem();
}

void  EAWebKitLib::AddTransportHandler(TransportHandler* pTH, const char16_t* pScheme)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::AddTransportHandler(pTH, pScheme);
}

void  EAWebKitLib::RemoveTransportHandler(TransportHandler* pTH, const char16_t* pScheme)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::RemoveTransportHandler(pTH, pScheme);
}

TransportHandler* EAWebKitLib::GetTransportHandler(const char16_t* pScheme)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetTransportHandler(pScheme);
}


void  EAWebKitLib::AddSocketTransportHandler(SocketTransportHandler* pSTH)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	if(pSTH)
	{
		if(!spSocketTransportHandler)
		{
			spSocketTransportHandler = pSTH;
			pSTH->Init();
		}
		else
		{
			EAW_ASSERT_MSG(false, "Adding SocketTransportHandler more than once. Not supported. Skipping the add");
		}
	}

}

void  EAWebKitLib::RemoveSocketTransportHandler(SocketTransportHandler* pSTH)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	if(spSocketTransportHandler == pSTH)
	{
		spSocketTransportHandler->Shutdown();
		spSocketTransportHandler = NULL;
	}
}

SocketTransportHandler* EAWebKitLib::GetSocketTransportHandler()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetSocketTransportHandler();
}

View* EAWebKitLib::CreateView()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::CreateView();
}

void EAWebKitLib::DestroyView(View* pView)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::DestroyView(pView);
}

void EAWebKitLib::SetRAMCacheUsage(const EA::WebKit::RAMCacheInfo& RAMCacheInfo)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::SetRAMCacheUsage(RAMCacheInfo);
}

void EAWebKitLib::GetRAMCacheUsage(EA::WebKit::RAMCacheUsageInfo& ramCacheUsageInfo)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetRAMCacheUsage(ramCacheUsageInfo);
}

bool EAWebKitLib::SetDiskCacheUsage(const EA::WebKit::DiskCacheInfo& diskCacheInfo)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::SetDiskCacheUsage(diskCacheInfo);
}

void EAWebKitLib::GetDiskCacheUsage(EA::WebKit::DiskCacheUsageInfo& diskCacheUsageInfo)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::GetDiskCacheUsage(diskCacheUsageInfo);
}

void EAWebKitLib::SetCookieUsage(const CookieInfo& cookieInfo)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::SetCookieUsage(cookieInfo);
}

void EAWebKitLib::RemoveCookies()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::RemoveCookies();
}

const char* EAWebKitLib::GetVersion()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetVersion();
}

JavascriptValue *EAWebKitLib::CreateJavascriptValue(View *view) 
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::CreateJavascriptValue(view);
}

void EAWebKitLib::DestroyJavascriptValue(JavascriptValue *v)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::DestroyJavascriptValue(v);
}

JavascriptValue *EAWebKitLib::CreateJavascriptValueArray(View *view, size_t count)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::CreateJavascriptValueArray(view, count);
}

void EAWebKitLib::DestroyJavascriptValueArray(JavascriptValue *array)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::DestroyJavascriptValueArray(array);
}

void EAWebKitLib::ClearMemoryCache()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::ClearMemoryCache();
}

void EAWebKitLib::NetworkStateChanged(bool isOnline)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::NetworkStateChanged(isOnline);
}

void EAWebKitLib::AddUserStyleSheet(const char8_t* userStyleSheet)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	if(!userStyleSheet || userStyleSheet[0] == '\0')
		return;

	if(WebCore::PageGroup* pageGroup = WebCore::PageGroup::pageGroup(kDefaultPageGroupName))
		pageGroup->addUserStyleSheetToWorld(WebCore::mainThreadNormalWorld(), userStyleSheet, WebCore::KURL(), nullptr, nullptr, WebCore::InjectInAllFrames);
	else
		EAW_ASSERT_MSG(false,"Unable to create page group. User style sheets won't work");
}

void EAWebKitLib::AddCookie(const char8_t* pHeaderValue, const char8_t* pURI)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::AddCookie(pHeaderValue, pURI);
}

uint16_t EAWebKitLib::ReadCookies(char8_t** rawCookieData, uint16_t numCookiesToRead)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::ReadCookies(rawCookieData, numCookiesToRead);
}

int32_t EAWebKitLib::LoadSSLCertificate(const uint8_t *pCACert, int32_t iCertSize)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);

#if ENABLE(DIRTYSDK_IN_DLL)
	int32_t result = ProtoSSLSetCACert(pCACert, iCertSize); 
	EAW_ASSERT(result > 0);
	return result;
#else
	EAW_ASSERT_MSG(false, "Invalid call. Loading certificates without enabling the DirtySDK code inside DLL\n");
	return -1;
#endif
}

void EAWebKitLib::ClearSSLCertificates()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);

#if ENABLE(DIRTYSDK_IN_DLL)
	ProtoSSLClrCACerts();
#else
	EAW_ASSERT_MSG(false, "Invalid call. Loading certificates without enabling the DirtySDK code inside DLL\n");
#endif
}

uint32_t EAWebKitLib::SetTextFromHeaderMapWrapper(const EASTLHeaderMapWrapper& headerMapWrapper, char* pHeaderMapText, uint32_t textCapacity)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::SetTextFromHeaderMapWrapper(headerMapWrapper, pHeaderMapText, textCapacity);

}

bool EAWebKitLib::SetHeaderMapWrapperFromText(const char* pHeaderMapText, uint32_t textSize, EASTLHeaderMapWrapper& headerMapWrapper, bool bExpectFirstCommandLine, bool bClearMap)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::SetHeaderMapWrapperFromText(pHeaderMapText, textSize, headerMapWrapper, bExpectFirstCommandLine, bClearMap);
}

void EAWebKitLib::SetPlatformSocketAPI(const EA::WebKit::PlatformSocketAPI& platformSocketAPI)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::SetPlatformSocketAPI(platformSocketAPI);
}

void EAWebKitLib::AddAllowedDomainInfo(const char8_t* allowedDomain, const char8_t* excludedPaths /* = 0 */)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::AddAllowedDomainInfo(allowedDomain, excludedPaths);
}

bool EAWebKitLib::CanNavigateToURL(const char8_t* url)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::CanNavigateToURL(url);
}

void EAWebKitLib::ReattachCookies(TransportInfo* pTInfo)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::ReattachCookies(pTInfo);
}

void EAWebKitLib::CookiesReceived(TransportInfo* pTInfo)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::CookiesReceived(pTInfo);
}

double EAWebKitLib::GetTime()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetTime();
}

EA::WebKit::Parameters& EAWebKitLib::GetParameters()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetParameters();
}

void EAWebKitLib::SetParameters(const EA::WebKit::Parameters& params)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::SetParameters(params);  
}

EA::WebKit::ThemeParameters& EAWebKitLib::GetThemeParameters()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return EA::WebKit::GetThemeParameters();
}

void EAWebKitLib::SetThemeParameters(const EA::WebKit::ThemeParameters& themeParams)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EA::WebKit::SetThemeParameters(themeParams);  
}

void JSLogCallback(const eastl::string8 &message, bool shouldAssert)
{
    if (EAWebKitClient *pClient = GetEAWebKitClient())
    {
        DebugLogInfo dli;
        dli.mpView = NULL;
        dli.mpUserData = NULL;
        dli.mpLogText = message.c_str();
        dli.mType = shouldAssert ? kDebugLogAssertion : kDebugLogWebCore;
        
        pClient->DebugLog(dli);
    }
#if _DEBUG //Print a log in debug in case the client does not exist yet (For example,during Init)
	else
	{
#if defined(_MSC_VER)
		OutputDebugStringA(message.c_str());
		OutputDebugStringA("\n");
#else
		printf("%s", message.c_str());
		printf("\n");
#endif
	}
#endif //_DEBUG
}


static void SetWebKitStatus(const WebKitStatus status) 
{
   sWebKitStatus = status; 
}

WebKitStatus GetWebKitStatus()
{
    return sWebKitStatus;
}

bool Init(AppCallbacks* appCallbacks, AppSystems* appSystems)
{
	if(appSystems)
	{
		// First thing to do is to SetAllocator so we don't worry about anything else trying to allocate memory before
		// this is done.
		if(appSystems->mAllocator)
			SetAllocator(appSystems->mAllocator);
		// Force the creation of allocator if one was not passed. This allows us to use the pointer directly instead of having to call GetAllocator all the time.
		// There is a good chance that compiler does not inline this function.
		GetAllocator();
		
		if(appSystems->mEAWebkitClient)
			SetEAWebKitClient(appSystems->mEAWebkitClient);
		if(appSystems->mFileSystem)
			SetFileSystem(appSystems->mFileSystem);
		if(appSystems->mTextSystem)
			SetTextSystem(appSystems->mTextSystem);
		if(appSystems->mThreadSystem)
			SetThreadSystem(appSystems->mThreadSystem);
	}

	// Force the creation of allocator if one was not passed. This allows us to use the pointer directly instead of having to call GetAllocator all the time. 
	// There is a good chance that compiler does not inline this function.
	GetAllocator();
	
	extern void SetPackageAllocators();
	SetPackageAllocators();
	
	//Next set up the logging to make sure that we don't miss any log.
	JSSetLogCallback(JSLogCallback);

	EAW_ASSERT_MSG( (GetWebKitStatus() == kWebKitStatusInactive), "Shutdown() needs to have been called or Init() was called twice");
	SetWebKitStatus(kWebKitStatusInitializing);

	// Make sure that Init is called only once per load/unload of the Dll.
	static bool initCalled = false;
	EAW_ASSERT_MSG( (!initCalled), "Library should not be initialized yet");
	initCalled = true;

	if(appCallbacks)
	{
		if(appCallbacks->timer)
			SetHighResolutionTimer(appCallbacks->timer);
		if(appCallbacks->stackBase)
			SetStackBaseCallback(appCallbacks->stackBase);
		if(appCallbacks->atomicIncrement)
			SetAtomicIncrementCallback(appCallbacks->atomicIncrement);
		if(appCallbacks->atomicDecrement)
			SetAtomicDecrementCallback(appCallbacks->atomicDecrement);
		if(appCallbacks->cryptRandomValue)
			SetCryptographicallyRandomValueCallback(appCallbacks->cryptRandomValue);
		
	}

	WebCore::InitializeLoggingChannelsIfNecessary();
	WebCore::ScriptController::initializeThreading();
	WebCore::SecurityOrigin::setLocalLoadPolicy(WebCore::SecurityOrigin::AllowLocalLoadsForLocalAndSubstituteData);
	WebPlatformStrategies::initialize();

#if ENABLE(EATEXT_IN_DLL) 
    if(!appSystems->mTextSystem)
        EA::WebKit::InitFontSystem();
#endif

#if USE(ACCELERATED_COMPOSITING)
    WebCore::pageCache()->setShouldClearBackingStores(true);
#endif

    SetWebKitStatus(kWebKitStatusActive);
    return true;
}

void Shutdown()
{
    EAW_ASSERT_MSG( (GetWebKitStatus() == kWebKitStatusActive), "Init() needs to have been called or Shutdown() was called twice");
    
	SET_AUTO_COLLECTOR_STACK_BASE();   // 9/4/09 - CSidhall - Added to get approximate stack base
	
	SetWebKitStatus(kWebKitStatusShuttingDown);
	WebCore::pageCache()->setCapacity(0);
	WebCore::pageCache()->releaseAutoreleasedPagesNow();    
#if ENABLE(INSPECTOR)
	WebCore::InspectorSettingsEA::finalize(); // This saves inspector settings
#endif
	WebCore::PageGroup::closeLocalStorage();
	if(WebCore::PageGroup* pageGroup = WebCore::PageGroup::pageGroup(kDefaultPageGroupName))
		pageGroup->removeUserStyleSheetsFromWorld(WebCore::mainThreadNormalWorld());

	WTF::waitForAllThreadsCompletion();			// Needed because if dll is freed before this, it would crash 

	if(ownsSocketTransportHandler)
	{
		spSocketTransportHandler->Shutdown();
		spSocketTransportHandler = NULL;
	}

	WebCore::ResourceHandleManager::finalize();	// Needed to free DirtySDK related resources
	EA::WebKit::GetTextSystem()->Shutdown();	// Needed to free EAText related resources 

#if ENABLE(EATEXT_IN_DLL) 
    EA::WebKit::ShutdownFontSystem();
#endif

	EA::WebKit::GetThreadSystem()->Shutdown(); // Needed to free any thread related resources. Call at last since some timer stuff uses it to query main thread.
	SetWebKitStatus(kWebKitStatusInactive);
}

void Destroy()
{
	//Note by Arpit Baldeva: WTF::fastDelete calls EAWebkit allocator under the hood. Since the first thing we do after loading the relocatable module is to 
	//create an instance, the application does not have a chance to setup the allocator. So if we use EAWebkit's default allocator for creating the 
	//concrete instance and later the application swaps out the allocator with a its own supplied allocator, we would end up freeing the ConcreteInstance
	//from an allocator that did not create it. The solution is to either use a global new/delete operator or create a static EAWebkitConcrete instance.
	//I chose to make a static in case we decide to overrride global new/delete for the relocatable module in the future. Then, we would avoid inadvertently
	//creating the instance from one allocator and freeing it using another.

	//WTF::fastDelete<EAWebkitConcrete>(this);
}


void SetEAWebKitClient(EAWebKitClient* pClient)
{
	EAW_ASSERT_MSG(!spEAWebKitClient, "EAWebKitClient already exists. Do not call SetEAWebKitClient more than once!");
	spEAWebKitClient = pClient;
}


EAWebKitClient* GetEAWebKitClient()
{
	return spEAWebKitClient;
}


double GetTime()
{
	return WTF::currentTime();
}

void SetHighResolutionTimer(EAWebKitTimerCallback timer)
{
	EAW_ASSERT_MSG(!gpTimerCallback, "Timer callback already exists. Do not call SetHighResolutionTimer more than once!");
	gpTimerCallback = timer;
}

EAWebKitTimerCallback GetHighResolutionTimer()
{
	return gpTimerCallback;
}

void SetStackBaseCallback(EAWebKitStackBaseCallback callback)
{
	EAW_ASSERT_MSG(!gpStackBaseCallback, "Stack base callback already exists. Do not call SetStackBaseCallback more than once!");
	gpStackBaseCallback = callback;
}

EAWebKitStackBaseCallback GetStackBaseCallback()
{
	return gpStackBaseCallback;
}

void SetAtomicIncrementCallback(EAWebKitAtomicIncrementCallback callback)
{
    gpAtomicIncrement = callback;
}

void SetAtomicDecrementCallback(EAWebKitAtomicIncrementCallback callback)
{
    gpAtomicDecrement = callback;
}

void SetCryptographicallyRandomValueCallback(EAWebKitCryptographicallyRandomValueCallback callback)
{
    gpCryptographicallyRandomValueCallback = callback;
}

void ReattachCookies(TransportInfo* pTInfo)
{
	if(!pTInfo)
		return;

	//Remove existing cookies from the transport headers
	EA::WebKit::HeaderMap::iterator it;
	while((it = GetHeaderMap(pTInfo->mHeaderMapOut)->find_as(EA_CHAR16("Cookie"), EA::WebKit::str_iless())) != GetHeaderMap(pTInfo->mHeaderMapOut)->end())
		GetHeaderMap(pTInfo->mHeaderMapOut)->erase(it);

	//Attach new cookies
	WebCore::ResourceHandleManager* pRHM = WebCore::ResourceHandleManager::sharedInstance();
	EAW_ASSERT(pRHM);

	EA::WebKit::CookieManager* pCM = pRHM->GetCookieManager();   
	pCM->OnHeadersSend(pTInfo);
}

void CookiesReceived(TransportInfo* pTInfo)
{
	if(!pTInfo)
		return;

	//Attach new cookies
	WebCore::ResourceHandleManager* pRHM = WebCore::ResourceHandleManager::sharedInstance();
	EAW_ASSERT(pRHM);

	EA::WebKit::CookieManager* pCM = pRHM->GetCookieManager();   
	pCM->OnHeadersRead(pTInfo);

}

bool SetDiskCacheUsage(const EA::WebKit::DiskCacheInfo& diskCacheInfo)
{
	if (!diskCacheInfo.mDiskCacheDirectory) 
		return false;

	const char8_t* pDiskCacheDir = diskCacheInfo.mDiskCacheDirectory;
    EA::IO::Path::PathString8 fullPath = GetFullPath(pDiskCacheDir, true);
	if(!fullPath.empty())
	{
		pDiskCacheDir = fullPath.c_str();
	}

	if(!EA::WebKit::GetFileSystem()->MakeDirectory(pDiskCacheDir))
	{
		EAW_ASSERT_FORMATTED(false , "Could not create directory for %s disk cache directory path \n",pDiskCacheDir);
		return false;
	}

	//11/17/2011 - Bug fix otherwise diskcache does not work with relative path for cache directory.
	EA::WebKit::DiskCacheInfo diskCacheInfoNew = diskCacheInfo;
	diskCacheInfoNew.mDiskCacheDirectory = pDiskCacheDir;
	return WebCore::ResourceHandleManager::sharedInstance()->SetDiskCacheParams(diskCacheInfoNew); 
}

void GetDiskCacheUsage(EA::WebKit::DiskCacheUsageInfo& diskCacheUsageInfo)
{
	WebCore::ResourceHandleManager::sharedInstance()->GetDiskCacheUsage(diskCacheUsageInfo); 
}

void SetCookieUsage(const EA::WebKit::CookieInfo& cookieInfo)
{
	const char8_t* pCookieFilePath = cookieInfo.mCookieFilePath;

	EA::IO::Path::PathString8 fullPath;
	if(pCookieFilePath)
	{
        fullPath.assign(GetFullPath(pCookieFilePath, true));
		if(!fullPath.empty())
		{
			pCookieFilePath = fullPath.c_str();
		}
	}

	WebCore::ResourceHandleManager* pRHM = WebCore::ResourceHandleManager::sharedInstance();
	EA::WebKit::CookieManager* pCM = pRHM->GetCookieManager();   
	CookieManagerParameters    params(pCookieFilePath, cookieInfo.mMaxIndividualCookieSize, 
									cookieInfo.mDiskCookieStorageSize, cookieInfo.mMaxCookieCount);
	pCM->SetParametersAndInitialize(params);
}

void SetRAMCacheUsage(const RAMCacheInfo& ramCacheInfo)
{
    sRamCacheInfo = ramCacheInfo;
	WebCore::memoryCache()->setCapacities(ramCacheInfo.mMinDeadBytes, ramCacheInfo.mMaxDeadBytes, ramCacheInfo.mTotalBytes);
}

void GetRAMCacheUsage(RAMCacheUsageInfo& ramCacheUsageInfo)
{
	WebCore::MemoryCache::Statistics stats = WebCore::memoryCache()->getStatistics();
	ramCacheUsageInfo.mImagesBytes = stats.images.size;
	ramCacheUsageInfo.mCssStyleSheetsBytes = stats.cssStyleSheets.size;
	ramCacheUsageInfo.mScriptsBytes = stats.scripts.size;
	ramCacheUsageInfo.mFontsBytes = stats.fonts.size;
}

void AddAllowedDomainInfo(const char8_t* allowedDomain, const char8_t* excludedPaths /* = 0 */)
{
	EAWebKitDomainFilter::GetInstance().AddAllowedDomainInfo(allowedDomain, excludedPaths);
}

bool CanNavigateToURL(const char8_t* pURL)
{
	return EAWebKitDomainFilter::GetInstance().CanNavigateToURL(pURL);
}


void RemoveCookies()
{
	WebCore::ResourceHandleManager* pRHM = WebCore::ResourceHandleManager::sharedInstance();
	EA::WebKit::CookieManager* pCM = pRHM->GetCookieManager();   
	pCM->RemoveCookies();
}

const char8_t* GetVersion()
{
	return EAWEBKIT_VERSION_S;
}

JavascriptValue *CreateJavascriptValue(View *view) 
{
    if(!view)
		return NULL;

	JSC::JSValue undefined = JSC::jsUndefined();
    JavascriptValue *v = new JavascriptValue(&undefined, view->GetJSExecState());
    return v;
}

void DestroyJavascriptValue(JavascriptValue *v)
{
    delete v;
}

JavascriptValue *CreateJavascriptValueArray(View *view, size_t count)
{
	if(!view)
		return NULL;
	
	void *exec = view->GetJSExecState();
    JavascriptValue *array = new JavascriptValue[count];
    for (size_t i = 0; i < count; ++i)
    {
        array[i].SetExec(exec);
    }

    return array;
}

void DestroyJavascriptValueArray(JavascriptValue *array)
{
    delete[] array;
}

void ClearMemoryCache()
{
	// This code is ported from Qt port. This would free up as much memory as possible.
	if (!WebCore::memoryCache()->disabled()) 
	{
		WebCore::memoryCache()->setDisabled(true);
		WebCore::memoryCache()->setDisabled(false);
	}

	int pageCapacity = WebCore::pageCache()->capacity();
	EAW_ASSERT_MSG(pageCapacity == 0, "We should not use page cache");
	// Setting size to 0, makes all pages be released.
	WebCore::pageCache()->setCapacity(0);
	WebCore::pageCache()->releaseAutoreleasedPagesNow();
	WebCore::pageCache()->setCapacity(pageCapacity);

	// Invalidating the font cache and freeing all inactive font data.
	WebCore::fontCache()->invalidate();

	// Empty the Cross-Origin Preflight cache
	WebCore::CrossOriginPreflightResultCache::shared().empty();

	// abaldeva: After freeing up the cache, we set the RAM Cache back to what user intends it to be.
	// If the user wants to disable the RAM Cache, they can call that API directly with 0 values.
	SetRAMCacheUsage(sRamCacheInfo);

}
void AddCookie(const char8_t* pHeaderValue, const char8_t* pURI)
{
	if(!pHeaderValue || !pURI)
		return;

	WebCore::ResourceHandleManager* pRHM = WebCore::ResourceHandleManager::sharedInstance();
	EA::WebKit::CookieManager* pCM = pRHM->GetCookieManager();   
	pCM->ProcessCookieHeader(pHeaderValue, pURI);
}

uint16_t ReadCookies(char8_t** rawCookieData, uint16_t numCookiesToRead)
{
	if(!rawCookieData)
		return 0;

	WebCore::ResourceHandleManager* pRHM = WebCore::ResourceHandleManager::sharedInstance();
	EA::WebKit::CookieManager* pCM = pRHM->GetCookieManager();   
	return pCM->EnumerateCookies(rawCookieData, numCookiesToRead);
}

void NetworkStateChanged(bool isOnline)
{
    WebCore::networkStateNotifier().setOnLine(isOnline);
}

void SetPlatformSocketAPI(const EA::WebKit::PlatformSocketAPI& platformSocketAPI)
{
#if ENABLE(DIRTYSDK_IN_DLL)  
	PlatformSocketAPICallbacks* socketCallbacks = GetPlatformSocketAPICallbacksInstance();
	if(socketCallbacks)
	{
		socketCallbacks->accept			=  platformSocketAPI.accept;
		socketCallbacks->bind			=  platformSocketAPI.bind;
		socketCallbacks->connect		=  platformSocketAPI.connect;
		socketCallbacks->gethostbyaddr	=  platformSocketAPI.gethostbyaddr;
		socketCallbacks->gethostbyname	=  platformSocketAPI.gethostbyname;
		socketCallbacks->dnslookup		=  platformSocketAPI.dnslookup;	
		socketCallbacks->getpeername	=  platformSocketAPI.getpeername;
		socketCallbacks->getsockopt		=  platformSocketAPI.getsockopt;
		socketCallbacks->listen			=  platformSocketAPI.listen;
		socketCallbacks->recv			=  platformSocketAPI.recv;
		socketCallbacks->recvfrom		=  platformSocketAPI.recvfrom;
		socketCallbacks->send			=  platformSocketAPI.send;
		socketCallbacks->sendto			=  platformSocketAPI.sendto;
		socketCallbacks->setsockopt		=  platformSocketAPI.setsockopt;
		socketCallbacks->shutdown		=  platformSocketAPI.shutdown;
		socketCallbacks->socket			=  platformSocketAPI.socket;
		socketCallbacks->close			=  platformSocketAPI.close;
		socketCallbacks->poll			=  platformSocketAPI.poll;
		socketCallbacks->getlasterror	=  platformSocketAPI.getlasterror;
	}
#else
	EAW_ASSERT_MSG(false, "Invalid call. Calling SetPlatformSocketAPI without enabling the DirtySDK code in DLL\n");
#endif
}

SocketTransportHandler* GetSocketTransportHandler()
{
	if(!spSocketTransportHandler)
	{
#if ENABLE(DIRTYSDK_IN_DLL)
		EAW_ASSERT_MSG(false, "No application supplied socket transport handler is found. Using the socket transport handler in the EAWebKit package.");
		spSocketTransportHandler = new EA::WebKit::SocketTransportHandlerDirtySDK();
		spSocketTransportHandler->Init();
		ownsSocketTransportHandler = true;
#endif
	}
	return spSocketTransportHandler;
}


///////////////////////////////////////////////////////////////////////
// Font / Text
///////////////////////////////////////////////////////////////////////

void SetTextSystem(ITextSystem* pTextSystem) 
{
	EAW_ASSERT_MSG(!spTextSystem, "TextSystem already exists. Do not call TextSystem more than once!");
	
	if(pTextSystem)
		pTextSystem->Init();

	spTextSystem = pTextSystem;

    // Set up the JS Core interface
    static EA::Internal::JSTextWrapper sJSTextWrapper;    
    JSSetTextInterface(&sJSTextWrapper);    // Just set it regardless, even if pTextSytem is null.
}

ITextSystem* GetTextSystem()
{
    EAW_ASSERT_MSG(spTextSystem, "Please set the text system with SetTextSystem()!");
    return spTextSystem;
}

///////////////////////////////////////////////////////////////////////
// Javascript Callstack Helper
///////////////////////////////////////////////////////////////////////
void JSCallstackCallback(const eastl::vector<eastl::string8> &names, const eastl::vector<eastl::string8> &args, const eastl::vector<int> &lines, const eastl::vector<eastl::string8> &urls)
{
    if (EAWebKitClient *pClient = GetEAWebKitClient())
    {
        const char **nameArray = new const char*[names.size()];
        const char **argArray = new const char*[args.size()];
        int *lineArray = new int[lines.size()];
        const char **urlArray = new const char*[urls.size()];

        EAW_ASSERT(names.size() == args.size() && names.size() == lines.size() && names.size() == urls.size());
        for (int i = 0; i < (int) names.size(); ++i)
        {
            nameArray[i] = names[i].c_str();
            argArray[i] = args[i].c_str();
            lineArray[i] = lines[i];
            urlArray[i] = urls[i].c_str();
        }

        ReportJSCallstackInfo info;
        info.mDepth = (int)names.size();
        info.mNames = nameArray;
        info.mArgs = argArray;
        info.mLines = lineArray;
        info.mUrls = urlArray;

        pClient->ReportJSCallstack(info);

        delete[] nameArray;
        delete[] argArray;
        delete[] lineArray;
        delete[] urlArray;
    }
}



///////////////////////////////////////////////////////////////////////
// Parameters
///////////////////////////////////////////////////////////////////////

Parameters::Parameters()
    : mpLocale(NULL) 
    , mpAcceptLanguageHttpHeaderValue(NULL) 
    , mpApplicationName(NULL)         
    , mpUserAgent(NULL)          
    , mMaxTransportJobs(16)
    , mHttpRequestResponseBufferSize(4096)
    , mPageTimeoutSeconds(30)
    , mHttpPipeliningEnabled(false)
    , mVerifySSLCert(true)
	, mTickHttpManagerFromNetConnIdle(true)
	, mSystemFontSize(13) 
	, mDefaultFontSize(16)           
	, mDefaultMonospaceFontSize(13)
	, mMinimumFontSize(1)                 
	, mMinimumLogicalFontSize(8)   
	, mSystemFontItalic(false)
	, mSystemFontBold(false)
	, mJavaScriptStackSize(128 * 1024)  // 128 K
	, mJavaScriptHeapWatermark (1 * 1024 * 1024) // 1 MB
	, mSmoothFontSize(18)
	, mFontFilterColorIntensity(255)
	, mEnableFontAlphaFilter(false)
	, mJavaScriptDebugOutputEnabled(false)         
    , mDefaultToolTipEnabled(true)   
    , mReportJSExceptionCallstacks(false)
	, mEnableProfiling(false)
    , mEnableImageCompression(false)
	, mIgnoreGammaAndColorProfile(false)
	, mEnableLowQualitySamplingDuringScaling(true)
    , mEnableRoundedCornerClip(false)
{
    // Set up default font names 
	EA::Internal::Strcpy(mSystemFont,			EA_CHAR16(""));                       // If no font name is set here (our default), webkit will decide on the font to use. 
    EA::Internal::Strcpy(mFontFamilyStandard,	EA_CHAR16("Times New Roman"));
	EA::Internal::Strcpy(mFontFamilySerif,		EA_CHAR16("Times New Roman"));
	EA::Internal::Strcpy(mFontFamilySansSerif,	EA_CHAR16("Arial"));
	EA::Internal::Strcpy(mFontFamilyMonospace,	EA_CHAR16("Courier New"));
	EA::Internal::Strcpy(mFontFamilyFantasy,	EA_CHAR16("Comic Sans MS"));      
	EA::Internal::Strcpy(mFontFamilyCursive,	EA_CHAR16("Comic Sans MS"));     
}

struct ParametersEx : public Parameters
{
    FixedString8_32  msLocale;
	FixedString8_32  msAcceptLanguageHttpHeaderValue;
    FixedString8_32  msApplicationName;
    FixedString8_128 msUserAgent;

    void operator=(const Parameters&);

    void Shutdown()
    {
        msLocale.set_capacity(0);
        msAcceptLanguageHttpHeaderValue.set_capacity(0);
        msApplicationName.set_capacity(0);
        msUserAgent.set_capacity(0);
    }
};

void ParametersEx::operator=(const Parameters& p)
{
    Parameters::operator=(p);
}

static ParametersEx& sParametersEx()
{
	static ParametersEx sParametersEx;
	return sParametersEx;
}

Parameters& GetParameters()
{
    return sParametersEx();
}

// Note: if adding a WebCore::Setting value, make sure to update the ApplyParamsToSettings() in WebPage.cpp so that te setting
// gets transfered to the page settings. 
void SetParameters(const Parameters& parameters)
{
    (sParametersEx()) = parameters;

    if(parameters.mpLocale)
        sParametersEx().msLocale = parameters.mpLocale;
    else
        sParametersEx().msLocale.clear();
    sParametersEx().mpLocale = sParametersEx().msLocale.c_str();

	if(parameters.mpAcceptLanguageHttpHeaderValue)
		sParametersEx().msAcceptLanguageHttpHeaderValue = parameters.mpAcceptLanguageHttpHeaderValue;
	else
		sParametersEx().msAcceptLanguageHttpHeaderValue.clear();
	sParametersEx().mpAcceptLanguageHttpHeaderValue = sParametersEx().msAcceptLanguageHttpHeaderValue.c_str();

    if(parameters.mpApplicationName)
        sParametersEx().msApplicationName = parameters.mpApplicationName;
    else
        sParametersEx().msApplicationName.clear();
    sParametersEx().mpApplicationName = sParametersEx().msApplicationName.c_str();

    if(parameters.mpUserAgent)
        sParametersEx().msUserAgent = parameters.mpUserAgent;
    else
        sParametersEx().msUserAgent.clear();
    sParametersEx().mpUserAgent = sParametersEx().msUserAgent.c_str();

    // JS 
    JSSetStackSize(parameters.mJavaScriptStackSize);
	JSSetHeapWatermark(parameters.mJavaScriptHeapWatermark);
    JSSetPrintExceptions(parameters.mJavaScriptDebugOutputEnabled);
	if(sParametersEx().mpLocale)
	{
		JSSetDefaultLocale(sParametersEx().mpLocale);
	}
    if (parameters.mReportJSExceptionCallstacks)
    {
        JSSetCallstackCallback(JSCallstackCallback);
    }

	WebCore::ResourceHandleManager::sharedInstance()->SetParams(parameters);
};

ThemeParameters::ThemeParameters()
    : mColorActiveSelectionBack(0xff3875d7)       
	, mColorActiveSelectionFore(0xffd4d4d4)
	, mColorInactiveSelectionBack(0xff3875d7)
	, mColorInactiveSelectionFore(0xffd4d4d4)
    , mColorActiveListBoxSelectionBack(0xff3875d7)
	, mColorActiveListBoxSelectionFore(0xffd4d4d4)
	, mColorInactiveListBoxSelectionBack(0xff3875d7)
	, mColorInactiveListBoxSelectionFore(0xffd4d4d4)
    , mBoxEnabledFillColor(0xFFF6F6F6)
    , mBoxDisabledFillColor(0xFFE0E0E0)
    , mCheckMarkColor(0xFF408040)
    , mHoverHighlightColor(0xFFE0A050)
    , mRadioButtonSize(11)
    , mCheckBoxSize(13)
    , mTextFieldBorderWidth(1)
    , mMenuListBorderWidth(1) 
	, mCustomFocusRingColor(0) 
	, mEnableFocusRingDraw(true)  
{
}


static ThemeParameters& sThemeParameters()
{
	static ThemeParameters sThemeParameters;
	return sThemeParameters;
}

ThemeParameters& GetThemeParameters()
{
    return sThemeParameters();
}

void SetThemeParameters(const ThemeParameters& themeParams)
{
    // Set Custom focus ring    
    WebCore::Color focusRingColor;  
    if (themeParams.mCustomFocusRingColor) 
    {
        focusRingColor.setRGB( (WebCore::RGBA32) themeParams.mCustomFocusRingColor); 
    }

    WebCore::RenderTheme::setCustomFocusRingColor(focusRingColor);   // RenderTheme checks if the Color is valid to determine if a custom color should be used.
}


// This function returns a valid full path if 
// 1. Base directory is specified.
// 2. RelativePath is correctly specified.
// Otherwise, it will return a default/empty pathstring.
EA::IO::Path::PathString8 GetFullPath(const char8_t* pRelativePath, bool useTempDirAsBasePath)
{
    // This function is written in such a manner is that if the GetBaseDirectory() does not return a valid path, it bypasses the code.
    // This ensures that we are backward compatible. 

    char baseDir[EA::IO::kMaxDirectoryLength];
    memset(baseDir, 0, EA::IO::kMaxDirectoryLength);

    bool baseFound = false;
    if (useTempDirAsBasePath)
    {
        baseFound = EA::WebKit::GetFileSystem()->GetTempDirectory(baseDir, EA::IO::kMaxDirectoryLength - 1);
    }
    else
    {
        baseFound = EA::WebKit::GetFileSystem()->GetDataDirectory(baseDir, EA::IO::kMaxDirectoryLength - 1);
    }

    if(baseFound)
    {
        EA::IO::Path::PathString8 fullPath;
        if(baseDir[0])
        {
            EA::IO::Path::PathString8 relativePath(pRelativePath);
            if(EA::IO::Path::IsRelative(relativePath))
            {
                fullPath.assign(baseDir);
                EA::IO::Path::Append(fullPath, relativePath);

                return fullPath;
            }
            else
            {
                EAW_ASSERT_FORMATTED(false , "Base directory specified but %s path is not relative. Base directory is ignored\n", pRelativePath);
            }
        }
        else //Just some safety code
        {
            EA::IO::Path::PathString8 relativePath(pRelativePath);
            if(EA::IO::Path::IsRelative(relativePath))
            {
                EAW_ASSERT_FORMATTED(false , "Base directory not specified but %s path is relative. \n", pRelativePath);
            }
        }
    }

    return EA::IO::Path::PathString8();
}

//+ 9/2/09 CSidhall - Added approximate stack base to avoid platform headers
AutoCollectorStackBase::AutoCollectorStackBase()
{
	// This grabs the current stack position
	void* dummy;
	gpCollectorStackBase = &dummy;    	
};

AutoCollectorStackBase::~AutoCollectorStackBase()
{
	// This is mostly for debug so we can track a problem with
	// a stackBase that was not set.
#ifdef _DEBUG    
	gpCollectorStackBase = 0; 
#endif
};

} // WebKit
} // EA


