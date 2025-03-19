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
// EAWebKit.h
// By Paul Pedriana
// Maintained by EAWebKit Team
///////////////////////////////////////////////////////////////////////////////


#ifndef EAWEBKIT_EAWEBKIT_H
#define EAWEBKIT_EAWEBKIT_H

#include <EAWebKit/EAWebKitConfig.h>
#include <EAWebKit/EAWebKitSystem.h>
#include <EAWebKit/EAWebKitDll.h>
namespace EA
{
namespace WebKit
{
struct PlatformSocketAPI;

enum WebKitStatus
{
    kWebKitStatusInactive,
    kWebKitStatusInitializing,
    kWebKitStatusActive,
    kWebKitStatusShuttingDown
};

struct RAMCacheInfo
{
	// Having a large enough RAM cache allows for faster draw (like when scrolling)
	// and faster page reload.
    uint32_t    mTotalBytes;    // Total number of bytes to be used for the RAM cache.  
    uint32_t    mMinDeadBytes;  // Min inactive data to keep in cache before releasing it. Normally 0 to release all when the space is needed. 
    uint32_t    mMaxDeadBytes;  // Max inactive data allowed to be stored in the cache.

	RAMCacheInfo()
		: mTotalBytes(8 * 1024 * 1024)
		, mMinDeadBytes(0)
        , mMaxDeadBytes(8/4 * 1024 * 1024)
	{
	}

	RAMCacheInfo(uint32_t totalBytes, uint32_t minDeadBytes, uint32_t maxDeadBytes)
        : mTotalBytes(totalBytes)
		, mMinDeadBytes(minDeadBytes)
        , mMaxDeadBytes(maxDeadBytes)
	{
	}
};

struct RAMCacheUsageInfo
{
	uint32_t mImagesBytes;			// Current RAM Cache memory used for images
	uint32_t mCssStyleSheetsBytes;	// Current RAM Cache memory used for css style sheets
	uint32_t mScriptsBytes;			// Current RAM Cache memory used for scripts
	uint32_t mFontsBytes;			// Current RAM Cache memory used for fonts. This is not same as cached glyphs that the text system may keep. 

	RAMCacheUsageInfo()
		: mImagesBytes(0)
		, mCssStyleSheetsBytes(0)
		, mScriptsBytes(0)
		, mFontsBytes(0)
	{

	}

};

struct DiskCacheInfo
{
	uint32_t		mDiskCacheSize;				// In bytes
	uint32_t		mMaxNumberOfCachedFiles;    // Max number of files that can be cached in the cache directory (+1 for the cache ini ctrl file). So a limit of 2 files will cache 2 files + 1 shared ctrl init file.   
	uint32_t		mMaxNumberOfOpenFiles;      // Max number of files that can keep stay open 
	uint32_t		mMinFileSizeToCache;        // Min file size in bytes to be cached.  This can prevent small 32 byte files from being cached for example.
	const utf8_t*	mDiskCacheDirectory;		// Full/Relative file path to writable directory. If relative, a valid full path should be returned when calling FileSystem::GetBaseDirectory(). SetDiskCacheUsage copies this string.

	DiskCacheInfo()
		: mDiskCacheSize(256 * 1024 * 1024) // Max size that the file cache can use
		, mMaxNumberOfCachedFiles(2048)     // Max files that can be cached (+1 for the init ctrl file)
		, mMaxNumberOfOpenFiles(24)         // Max number of files that can stay open
		, mMinFileSizeToCache(1024)         // Min file size in bytes to be cached 
		, mDiskCacheDirectory(0)            // Path to the file cache
	{
	}

	DiskCacheInfo(uint32_t diskCacheSize, const utf8_t* cacheDiskDirectory, uint32_t maxNumberOfCachedFiles = 2048, uint32_t maxNumberOfOpenFiles = 24, uint32_t minFileSizeToCache = 1024)
		: mDiskCacheSize(diskCacheSize)
		, mMaxNumberOfCachedFiles(maxNumberOfCachedFiles)
		, mMaxNumberOfOpenFiles(maxNumberOfOpenFiles)
		, mMinFileSizeToCache(minFileSizeToCache)
		, mDiskCacheDirectory(cacheDiskDirectory)
	{
	}

};

struct DiskCacheUsageInfo
{
	uint32_t		mCurrentNumberOfCachedFiles;    // Current number of files that are cached in the cache directory
	uint32_t		mCurrentNumberOfOpenFiles;		// Current number of files that are open 

	DiskCacheUsageInfo()
		: mCurrentNumberOfCachedFiles(0)
		, mCurrentNumberOfOpenFiles(0)
	{

	}
};
struct CookieInfo
{
	uint32_t		mMaxIndividualCookieSize;	// Should usually be at least 4096. The usable space is mMaxIndividualCookieSize-1.
	uint32_t		mDiskCookieStorageSize;		// Should usually be at least 32768.
	uint16_t		mMaxCookieCount;			// Max number of concurrent cookies. Should usually be at least 16. Set to zero to clear and disable cookies.
	const utf8_t*	mCookieFilePath;			// Full/Relative file path to writable directory. If relative, a valid full path should be returned when calling FileSystem::GetBaseDirectory(). SetCookieUsage copies this string.      

	CookieInfo()
		: mMaxIndividualCookieSize(4096)
		, mDiskCookieStorageSize(32768)
		, mMaxCookieCount(256)
		, mCookieFilePath(0)

	{

	}
	CookieInfo(const utf8_t* cookieFilePath, uint32_t maxIndividualCookieSize, uint32_t diskCookieStorageSize, uint16_t maxCookieCount)
		: mMaxIndividualCookieSize(maxIndividualCookieSize)
		, mDiskCookieStorageSize(diskCookieStorageSize)
		, mMaxCookieCount(maxCookieCount)
		, mCookieFilePath(cookieFilePath)
	{

	}
};

const size_t kFontNameCapacity = 32;
struct Parameters
{
	const char8_t*      mpLocale;                           // Defaults to NULL, which means "en-us". The SetParameters function copies this string, mpLocale doesn't need to persist. Currently unused.
	const char8_t*		mpAcceptLanguageHttpHeaderValue;    // Defaults to NULL, which means the "Accept-Language" header is not sent. If present, this is the "value" part of the header. This is used when making an http(s) request.The SetParameters function copies this string, mpAcceptLanguage doesn't need to persist.
	const char8_t*      mpApplicationName;                  // Defaults to NULL, which means "EAWebKit". The SetParameters function copies this string, mpApplicationName doesn't need to persist.

	// Transport settings:
	const char8_t*      mpUserAgent;                    // Defaults to NULL, which means "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.3 (KHTML, like Gecko) Safari/535.3 EAWebKit/"<EAWebKit Version>. The SetParameters function copies this string, mpUserAgent doesn't need to persist. See http://www.useragentstring.com/
	uint32_t            mMaxTransportJobs;              // Defaults to 16. Specifies maximum number of concurrent transport jobs (e.g. HTTP requests).
	uint32_t			mHttpRequestResponseBufferSize; // Defaults to 4096. Number of bytes that a HTTP request/response handle has for transaction with server. This is only for request/response headers and does not put any limit on the actual resource size(say a css file).
	uint32_t            mPageTimeoutSeconds;            // Defaults to kPageTimeoutDefault. Page load timeout, in seconds.
	bool				mHttpPipeliningEnabled;		    // Defaults to false. If enabled, the current implementation increases load times with slightly lower memory usage.
	bool                mVerifySSLCert;                 // Defaults to true. If true then we do SSL/TLS peer verification via security certificates. You should set this to false only if debugging non-shipping builds. Was called mVerifyPeersEnabled in 1.x.
	bool				mTickHttpManagerFromNetConnIdle;// Defaults to true (existing behavior). If true, NetConnIdle call in DirtySDK updates HttpManager used by this library. This results in slightly improved performance. The reason to turn it false would be if you are ticking EAWebKit and DirtySDK on different threads.
	//+ Start of WebCore::Settings section (these values get transfered down to the WebCore Settings but only for a new webframe)
	// Font settings
	uint32_t   mSystemFontSize;            // Defaults to 13                 
	uint32_t   mDefaultFontSize;           // Defaults to 16.
	uint32_t   mDefaultMonospaceFontSize;  // Defaults to 13.
	uint32_t   mMinimumFontSize;           // Defaults to 1.
	uint32_t   mMinimumLogicalFontSize;    // Defaults to 8.
	char16_t   mFontFamilyStandard[kFontNameCapacity];    // Defaults to "Times New Roman". These should be set to a font you have installed with the EAText FontServer.
	char16_t   mFontFamilySerif[kFontNameCapacity];       // Defaults to "Times New Roman".
	char16_t   mFontFamilySansSerif[kFontNameCapacity];   // Defaults to "Arial".
	char16_t   mFontFamilyMonospace[kFontNameCapacity];   // Defaults to "Courier New".
	char16_t   mFontFamilyCursive[kFontNameCapacity];     // Defaults to "Comic Sans MS".
	char16_t   mFontFamilyFantasy[kFontNameCapacity];     // Defaults to "Comic Sans MS".
    char16_t   mSystemFont[kFontNameCapacity];            // Defaults to 0. This is the font that appears in form elements such as listboxes.
	bool       mSystemFontItalic;                         // Defaults to false. Italic if true. 
	bool       mSystemFontBold;                           // Defaults to false. Bold if true.
	//- End of WebCore::Settings section 

	//Note by Arpit Baldeva: JavaScriptStackSize defaults to 128 KB. The core Webkit trunk allocates 2MB by default (well, they don't allocate but assume that the platform has on-demand commit capability) at the time of writing. This is not suitable for consoles with limited amount of memory and without on-demand commit capability.
	// The user can tweak this size and may be get around by using a smaller size that fits their need. If the size is too small, some JavaScript code may not execute. This would fire an assert in the debug builds.
	uint32_t    mJavaScriptStackSize;
	uint32_t	mJavaScriptHeapWatermark; // PC Only(Ignored on Consoles as default value is sufficient). The watermark at which JSCore engine starts to recollect blocks. Higher watermark means more speed of execution. 1 MB by default. 

	// Font smoothing size
	uint32_t    mSmoothFontSize;                // Default to 18.  If 0, all font sizes are smooth (anti-aliased), including bold and italic. Results can vary depending on font family and sizes (better in general for larger fonts).
	uint32_t    mFontFilterColorIntensity;      // Default is 48.  0-255 range.  If mEnableFontAlphaFilter is true, it will filter any pen color that has a color channel <= to this intentsity value.  It is to avoid filtering bright colors (e.g white) which can sometimes look faded if alpha filtered.
	bool        mEnableFontAlphaFilter;         // Default is false.  If set, it will lower the alpha values of glyphs (mostly used to counter blur from smoothing) if the pen color channels are under mFontFilterColorIntensity value.

	bool mJavaScriptDebugOutputEnabled;     // Defaults to false. If enabled, this will print the results of console.log and any javascript errors/exceptions to TTY
	bool mDefaultToolTipEnabled;            // Default to true.  Allows the display of the default tool tip text. 

	
    bool mReportJSExceptionCallstacks;			// Defaults to false.
	bool mEnableProfiling;						// Disabled by default. If enabled, EAWebKitClient::ViewProcessStatus is called with profiling info.
	bool mEnableImageCompression;               // Defaults to false.  If enabled, it will compress decoded images saving runtime memory at the expense of decoding processing.
	bool mIgnoreGammaAndColorProfile;			// Defaults to false. If enabled, the gamma correction and color profile for images is skipped resulting in faster decoding.
	bool mEnableLowQualitySamplingDuringScaling;// Default to true. If enabled, the image is sampled at low quality before it comes to rest. The image remains at low quality for at least 50 ms after scaling is done. Useful for good performance if your page has heavy zoom/pan usage(for example, maps.google.com) 
    bool mEnableRoundedCornerClip;              // Enables detailed rounded corner clipping. This can however impact performance so off by default. 

    Parameters();
private:
	// Disallow compiler generated copy ctor. The embedder should take a reference using GetParameters().
	Parameters(const Parameters&);
};


// These are optional settings for various buttons, scroll bars and selections colors
// Note that most of these do NOT have a direct mapping in terms of CSS properties as WebKit expects them to specified by the OS. Thus, these
// cannot be overriden by user supplied style sheet.
// In general, the users should try to use CSS styling mechanism to the extent possible for making WebKit rendered content similar to the 
// rest of the app in terms of look and feel.
struct ThemeParameters
{
	// Selection colors (e.g highlight text for cut&paste)
	uint32_t mColorActiveSelectionBack;       
	uint32_t mColorActiveSelectionFore;
	uint32_t mColorInactiveSelectionBack;
	uint32_t mColorInactiveSelectionFore;

	// List/Combo/Select dropdown box 
	uint32_t mColorActiveListBoxSelectionBack;   // Default Background color for the selected item in a List/Combo/Select dropdown box       
	uint32_t mColorActiveListBoxSelectionFore;   // Default color for the selected text in a List/Combo/Select dropdown box 
	uint32_t mColorInactiveListBoxSelectionBack; // Default Background color for the unselected item in a List/Combo/Select dropdown box
	uint32_t mColorInactiveListBoxSelectionFore; // Default color for the unselected text in a List/Combo/Select dropdown box

	// Buttons/Text Fields/Radio/Check Mark etc.
	uint32_t mBoxEnabledFillColor;   // For the inside of radio and check boxes when enabled.
	uint32_t mBoxDisabledFillColor;  // For the inside of radio and check boxes when not enabled.
	uint32_t mCheckMarkColor;        // Used for radio and check marks
	uint32_t mHoverHighlightColor;   // Used for hover highlights
	uint32_t mRadioButtonSize;       // Radio button size
	uint32_t mCheckBoxSize;          // Check mark size.  In 1.x we used the same size for radio and checkbox.
	uint32_t mTextFieldBorderWidth;  // Border for text field boxes. Setting to 2 will give a more 3D feel. 
	uint32_t mMenuListBorderWidth;   // Border for menu list boxes. Setting to 2 will give a more 3D feel. 

	// ScrollBar 
	// ScollBar can be completely customized using CSS.

    // Custom focus ring color
	uint32_t mCustomFocusRingColor;     // optional custom color for the focus ring. (0 by default which disables using a custom color). 
	bool     mEnableFocusRingDraw;      // Enables the focus ring draw (true by default) 

	ThemeParameters();
};


}
}

namespace EA
{
namespace WebKit
{
class Allocator;
class FileSystem;
class EAWebKitClient;
class View;
class EASTLFixedString16Wrapper;
class EASTLFixedString8Wrapper;
class EASTLHeaderMapWrapper;
class TransportHandler;
class TransportInfo;
class SocketTransportHandler;
class JavascriptValue;
class ITextSystem;
class IThreadSystem;

// Callbacks
typedef double	(*EAWebKitTimerCallback)();
typedef void*	(*EAWebKitStackBaseCallback)();
typedef int     (*EAWebKitAtomicIncrementCallback)(int volatile *addend);
typedef int     (*EAWebKitAtomicDecrementCallback)(int volatile *addend);
typedef void    (*EAWebKitCryptographicallyRandomValueCallback)(unsigned char *buffer, size_t length);

struct AppCallbacks
{
EAWebKitTimerCallback							timer;
EAWebKitStackBaseCallback						stackBase;
EAWebKitAtomicIncrementCallback					atomicIncrement;
EAWebKitAtomicDecrementCallback					atomicDecrement;
EAWebKitCryptographicallyRandomValueCallback	cryptRandomValue;
};

struct AppSystems
{
EA::WebKit::Allocator* mAllocator;
EA::WebKit::FileSystem* mFileSystem;
EA::WebKit::ITextSystem* mTextSystem;
EA::WebKit::IThreadSystem* mThreadSystem;
EA::WebKit::EAWebKitClient* mEAWebkitClient;
};

class EAWebKitLib
{
public:
	// APIs related to the library setup/tear down
	// 12/05/2011 - The Init function now accepts the callbacks and system overrides as part of it. 
	virtual bool Init(AppCallbacks* appCallbacks = NULL, AppSystems* appSystems = NULL);
	virtual void Shutdown();
	virtual void Destroy();

	// Accessors
	virtual EAWebKitClient* GetEAWebKitClient();
	virtual Allocator*		GetAllocator();
	virtual FileSystem*		GetFileSystem();
	virtual ITextSystem*	GetTextSystem();
	virtual IThreadSystem*	GetThreadSystem();

	// APIs related to EA::WebKit::View
	virtual View*	CreateView();
	virtual void	DestroyView(View* pView);
	
	// APIs related to EAWebKit run time preferences
	virtual void				SetParameters(const Parameters& params);
	virtual Parameters&			GetParameters();
	virtual void				SetRAMCacheUsage(const RAMCacheInfo& ramCacheInfo);
    virtual void				GetRAMCacheUsage(RAMCacheUsageInfo& ramCacheUsageInfo);
   	virtual bool				SetDiskCacheUsage(const DiskCacheInfo& diskCacheInfo);
	virtual void				GetDiskCacheUsage(DiskCacheUsageInfo& diskCacheUsageInfo);
	virtual void				SetCookieUsage(const CookieInfo& cookieInfo);
	virtual void				SetThemeParameters(const ThemeParameters& themeParams);
	virtual ThemeParameters&	GetThemeParameters();

	// APIs related to Domain filtering system. Useful if you don't have full control over content pages(For example, a third party payment processor site)
	virtual void AddAllowedDomainInfo(const char8_t* allowedDomain, const char8_t* excludedPaths = 0);
	virtual bool CanNavigateToURL(const char8_t* url);
	
	// APIs related to Transport handlers
	virtual void              AddTransportHandler(TransportHandler* pTH, const char16_t* pScheme);
	virtual void              RemoveTransportHandler(TransportHandler* pTH, const char16_t* pScheme);
	virtual TransportHandler* GetTransportHandler(const char16_t* pScheme);

	//APIs related to Socket transport (For Web Socket support)
	virtual void					AddSocketTransportHandler(SocketTransportHandler* pSTH);
	virtual void					RemoveSocketTransportHandler(SocketTransportHandler* pSTH);
	virtual SocketTransportHandler* GetSocketTransportHandler();

    // APIs related to Javascript
    virtual JavascriptValue *CreateJavascriptValue(View *view);
    virtual void DestroyJavascriptValue(JavascriptValue *v);
    virtual JavascriptValue *CreateJavascriptValueArray(View *view, size_t count);
    virtual void DestroyJavascriptValueArray(JavascriptValue *args);

	// Misc. Runtime APIs
	virtual void ClearMemoryCache();// Call this to Free up Cache if running low on memory.
	virtual void NetworkStateChanged(bool isOnline); //Call this to indicate any change in network status. 
	
	// Add a user style sheet that affects all rendered pages.
	// Example 1 - Adding ":link, :visited { text-decoration: line-through ! important; }" will line-through all the links on all pages even if styled by the page.
	// Example 2 - Adding ":link, :visited { text-decoration: line-through;}" will line-through all the links that are not styled on the pages.
	virtual void AddUserStyleSheet(const char8_t* userStyleSheet);



	// APIs related to mostly debugging
	virtual const char8_t*	GetVersion();

	//
	// Add newer APIs above this.
	//


	////////////////////////////////////////////////////////////////////////

	//
	//
	// Normally, an application does not need following two unless they want to manage cookie storage themselves. 
	// For example, by combining cookies from other browsers.
	
	//An application can pass cookie text(in the standard http headers format without the Set-Cookie/Set-Cookie2 key) and the associated URL to add a 
	//cookie explicitly.  
	virtual void		AddCookie(const char8_t* pHeaderValue, const char8_t* pURI);
	// An application can pass in the buffer and read the cookies data.
	// The return value indicates the number of cookies actually read.
	// The buffer is expected to have enough capacity as specified using mMaxCookieCount and mMaxIndividualCookieSize in the CookieInfo structure.
	// so rawCookieData[mMaxCookieCount][mMaxIndividualCookieSize].
	virtual uint16_t	ReadCookies(char8_t** rawCookieData, uint16_t numCookiesToRead);
	//
	//
	////////////////////////////////////////////////////////////////////////

	
	
	
	
	
	
	
	
	
	
	
	////////////////////////////////////////////////////////////////////////
	//
	//
	//
	//
	// Following APIs are not expected to be called by the users of EAWebKit. They exist for various reasons. 
	// Some of them are used by EAWebKit internally.
	//
	//
	//
	// Following would load SSL certificates on the Dll side transport handler. If your application supplies 
	// its own transport handler, you need to load the certificate on the application side.
	virtual int32_t LoadSSLCertificate(const uint8_t *pCACert, int32_t iCertSize);
	virtual void	ClearSSLCertificates();
	virtual void	SetPlatformSocketAPI(const EA::WebKit::PlatformSocketAPI& platformSocketAPI);

	virtual uint32_t	SetTextFromHeaderMapWrapper(const EASTLHeaderMapWrapper& headerMapWrapper, char8_t* pHeaderMapText, uint32_t textCapacity);
	virtual bool		SetHeaderMapWrapperFromText(const char8_t* pHeaderMapText, uint32_t textSize, EASTLHeaderMapWrapper& headerMapWrapper,  bool bExpectFirstCommandLine, bool bClearMap);
	virtual double		GetTime();


	virtual void ReattachCookies(TransportInfo* pTInfo);
	virtual void CookiesReceived(TransportInfo* pTInfo);
	virtual void RemoveCookies();

	virtual ~EAWebKitLib()
	{
	}
};

}
}



#endif // EAWEBKIT_EAWEBKIT_H
