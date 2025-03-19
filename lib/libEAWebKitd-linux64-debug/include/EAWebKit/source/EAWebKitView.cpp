/*
Copyright (C) 2008, 2009, 2010, 2011, 2012 Electronic Arts, Inc.  All rights reserved.

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

#include "config.h"
#include <EAWebKit/EAWebKitView.h>
#include <EAWebKit/EAWebKitClient.h> 
 
#include "Page.h" 
#include "SharedTimer.h"
#include "ResourceHandleManagerEA.h"
#include "MainThread.h"
#include "WebPageClientEA.h"
#include "Settings.h"
#include "WebFrame.h"
#include "WebPage_p.h"
#include "WebFrame_p.h"
#include "FrameView.h"
#include "RefPtrCairo.h"
#include "TextPopupEA.h"
#include <HTMLNames.h>
#include <HTMLInputElement.h>
#include <HTMLTextAreaElement.h>
#include <FileSystem.h>
#include <internal/include/EAWebKitString.h>
#include "DOMTimeStamp.h"
#include "PageClientEA.h"
#include "TileEA.h"
#include "TextureMapperEA.h"

#include <EAWebKit/EAWebKitSystem.h>
#include <internal/include/EAWebKit_p.h>
#include <internal/include/EAWebKitAssert.h>
#include <internal/include/EAWebkitEASTLHelpers.h>  
#include <internal/include/EAWebKitFPUPrecision.h> 
#include <internal/include/InputBinding/EAWebKitUtils.h>
#include <internal/include/EAWebKitViewNavigationDelegate.h>

#include "WebInspector.h"



// Note by Arpit Baldeva:
// We use SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended); in ALL the virtual functions (To keep guideline simple)
// in EA::WebKit::View(). 
// This is because unexpected part of EAWebKit code may require extended precision and result in not fairly reproducible bugs.

namespace EA { namespace WebKit
{

//
// Private implementation
//
namespace 
{
struct OverlaySurfaceInfo
{
    ISurface* mpSurface;        // The overlay surface used.
    WebCore::IntRect mRect;     // The overlay rect which is used for region detection.

    OverlaySurfaceInfo(ISurface* pS, int x, int y, int w, int h)
    : mpSurface(pS)
    , mRect(x, y, w, h) 
    {
        // Do nothing.
    }
};
}

//
// ViewPrivate - Data-only structure that holds the implementation details for the View class
// so that we don't have to wrap things like eastl containers that aren't exported.
//
// ViewPrivate should not contain functions, it's only used to store private data
// for the View class.
struct ViewPrivate 
{
public:
	ViewPrivate(View *view)
	: view(view)
	, page(0)
    , mDisplaySurface(NULL)
	, mpUserData(NULL)
    , mHardwareRenderer(NULL)
	, mUsingTiledBackingStore(true)
	, mRemoveNonVisibleTiles(true)
	, mTileSize(512)
    , mBackgroundColor(0xffffffff)
    , mpOverlayInputClient(NULL)
	, mNavDelegate(view)
	, mEmulatingConsoleOnPC(false)
    , mBeingDebugged(false)
    , mMouseButtonPressedId(kMouseNoButton)
#if ENABLE(REQUEST_ANIMATION_FRAME)
    , mNeedsAnimation(false)
#endif
    , mToolTip(view)
    , mCachedCallstackNameArray(NULL)
    , mCachedCallstackArgArray(NULL)
    , mCachedCallstackLineArray(NULL)
    , mCachedCallstackUrlArray(NULL)
	, mInitialized(false)
	, mDrawDebugVisuals(false)
	{
		EAW_ASSERT(view);
	}

    void OverlayChangeNotify(void)
    {
        // Notify clients.
        if (mpOverlayInputClient)
        {
            mpOverlayInputClient->OnFocusChangeEvent(false);  
        }
        mToolTip.hide();
    };

	View *view;
	WebPage *page;

	FixedString16_128 mURL;
	FixedString16_128 mTitle;

    ISurface *mDisplaySurface;
	void *mpUserData;
    IHardwareRenderer *mHardwareRenderer;
	bool mUsingTiledBackingStore;
	bool mRemoveNonVisibleTiles;
	int mTileSize;

    typedef eastl::vector<WebCore::IntRect> DirtyRegions;
	DirtyRegions mDirtyRegions;

    typedef eastl::map<eastl::string, IJSBoundObject*> JSObjectMap;
    JSObjectMap mBoundObjects;

    uint32_t mBackgroundColor;

    IOverlayInputClient* mpOverlayInputClient;           // Redirects input to the overlay when set.
    
	typedef eastl::vector<OverlaySurfaceInfo> OverlaySurfaces;
	OverlaySurfaces mOverlaySurfaces;  // Stores the active overlays.
	
	ViewNavigationDelegate	mNavDelegate;
	EA::WebKit::IntPoint mCursorPos;
	bool mEmulatingConsoleOnPC;
    bool mBeingDebugged;
    uint32_t mMouseButtonPressedId;                       // Capture the mouse button id if still pressed so we can pass it to the mouse move event (needed for drag detection).
#if ENABLE(REQUEST_ANIMATION_FRAME)
    bool mNeedsAnimation;
#endif
    WebCore::TextPopup mToolTip;

    // Callstack Caching - Greatly simplifies external callstack API.
    eastl::vector<eastl::string8> mLastCallstackNames;
    eastl::vector<eastl::string8> mLastCallstackArguments;
    eastl::vector<int> mLastCallstackLines;
    eastl::vector<eastl::string8> mLastCallstackUrls;
#if ENABLE(TILED_BACKING_STORE)
	typedef eastl::list<WebCore::TileEA*> Tiles;
	Tiles mTiles;
#endif	
#if USE(ACCELERATED_COMPOSITING)
	typedef eastl::list<WebCore::BitmapTextureEA*> Textures;
	Textures mTextures;
#endif

    const char **mCachedCallstackNameArray;
    const char **mCachedCallstackArgArray;
    int *mCachedCallstackLineArray;
    const char **mCachedCallstackUrlArray;

    eastl::string16 mEncodedURICache;
	bool mInitialized;

	bool mDrawDebugVisuals;
};

void View::ScheduleAnimation(void)
{
#if ENABLE(REQUEST_ANIMATION_FRAME)
    d->mNeedsAnimation = true;
#endif
}

bool View::HardwareAccelerated(void)
{
    return d->mHardwareRenderer != NULL;
}

void View::Paint(void) 
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	
    NOTIFY_PROCESS_STATUS(kVProcessTypePaint, EA::WebKit::kVProcessStatusStarted, this);

    if (d->page)
	{ 
        WebFrame *frame = d->page->mainFrame();
        if (frame) 
		{
            WebCore::Frame *coreFrame = WebFramePrivate::core(frame);
            if (coreFrame) 
			{
                WebCore::FrameView *coreView = coreFrame->view();
                if (coreView && coreFrame->contentRenderer()) 
				{
#if ENABLE(REQUEST_ANIMATION_FRAME)
                    if (d->mNeedsAnimation)
                    {
                        d->mNeedsAnimation = false;
                        NOTIFY_PROCESS_STATUS(kVProcessTypeAnimation, EA::WebKit::kVProcessStatusStarted, this);
                        coreView->serviceScriptedAnimations(WebCore::convertSecondsToDOMTimeStamp(currentTime()));
                        NOTIFY_PROCESS_STATUS(kVProcessTypeAnimation, EA::WebKit::kVProcessStatusEnded, this);
                    }
#endif

                    NOTIFY_PROCESS_STATUS(kVProcessTypeLayout, EA::WebKit::kVProcessStatusStarted, this);
                    coreView->updateLayoutAndStyleIfNeededRecursive();
                    NOTIFY_PROCESS_STATUS(kVProcessTypeLayout, EA::WebKit::kVProcessStatusEnded, this);

					if(!HardwareAccelerated()) //Software rendering path
					{
						if (!d->mDirtyRegions.empty()) 
						{
							ViewUpdateInfo info;
							info.mpView = this;
							info.mpUserData = d->mpUserData;
							info.mStage = ViewUpdateInfo::Begin;

							//AddDirtyRegion(IntRect(IntPoint(0, 0), GetSize())); //Uncomment as a debug tool to repaint entire screen

							// Translate the dirty regions into something we can export outside the DLL.
							eastl::vector<IntRect> eaDirtyRegions(d->mDirtyRegions.size());
							for (int i = 0; i < (int) d->mDirtyRegions.size(); ++i)
							{
								eaDirtyRegions[i] = d->mDirtyRegions[i];
							}

							info.mDirtyRegions = eaDirtyRegions.data();
							info.mDirtyRegionCount = eaDirtyRegions.size();

							EAWebKitClient *pClient = GetEAWebKitClient();
							if (pClient) 
							{
								NOTIFY_PROCESS_STATUS(kVProcessTypeBeginViewUpdate, EA::WebKit::kVProcessStatusStarted, this);
								pClient->ViewUpdate(info);
								NOTIFY_PROCESS_STATUS(kVProcessTypeBeginViewUpdate, EA::WebKit::kVProcessStatusEnded, this);
							}

							frame->render(d->mDisplaySurface, d->mDirtyRegions);
							PaintOverlays();

							if (pClient) 
							{
								info.mStage = ViewUpdateInfo::End;

								NOTIFY_PROCESS_STATUS(kVProcessTypeEndViewUpdate, EA::WebKit::kVProcessStatusStarted, this);
								pClient->ViewUpdate(info);
								NOTIFY_PROCESS_STATUS(kVProcessTypeEndViewUpdate, EA::WebKit::kVProcessStatusEnded, this);
							}
							d->mDirtyRegions.clear();
						}
					}
					else //Hardware rendering path
					{
#if USE(ACCELERATED_COMPOSITING)
						NOTIFY_PROCESS_STATUS(kVProcessTypeSyncLayers, EA::WebKit::kVProcessStatusStarted, this);
						static_cast<WebCore::PageClientWebView*>(Page()->d->client.get())->syncLayers();
						NOTIFY_PROCESS_STATUS(kVProcessTypeSyncLayers, EA::WebKit::kVProcessStatusEnded, this);
						
						d->mHardwareRenderer->BeginPainting();

#if ENABLE(TILED_BACKING_STORE)
                            if(d->mUsingTiledBackingStore)
							{
								if (WebCore::TiledBackingStore *backingStore = WebFramePrivate::core(d->page->mainFrame())->tiledBackingStore())
								{
									NOTIFY_PROCESS_STATUS(kVProcessTypePaintTilesCPU, EA::WebKit::kVProcessStatusStarted, this);
									// Following paints the web page content into the tiles. The analogous call when using software renderer is
									// frame->render(d->mDisplaySurface, d->mDirtyRegions); from few lines above.
									// This also populates d->mDirtyRegions.
									backingStore->UpdateTilesIfNeeded();
									NOTIFY_PROCESS_STATUS(kVProcessTypePaintTilesCPU, EA::WebKit::kVProcessStatusEnded, this);
								}
							}
							else
							{
								frame->render(d->mDisplaySurface, d->mDirtyRegions);
							}
#endif //ENABLE(TILED_BACKING_STORE)
						NOTIFY_PROCESS_STATUS(kVProcessTypeHardwareRender, EA::WebKit::kVProcessStatusStarted, this);
						// If hardware acceleration is enabled we always want to call frame->renderAccelerated
						// so that the compositing commands get sent to the GPU.
						frame->renderAccelerated(d->mDirtyRegions);
						PaintOverlaysAccelerated();
						d->mDirtyRegions.clear();
						d->mHardwareRenderer->EndPainting();

						NOTIFY_PROCESS_STATUS(kVProcessTypeHardwareRender, EA::WebKit::kVProcessStatusEnded, this);


#else
						EAW_ASSERT_MSG(false, "Trying to use hardware acceleration path without the supporting code");
#endif
					}
                }
            }
        }
    }
    NOTIFY_PROCESS_STATUS(kVProcessTypePaint, EA::WebKit::kVProcessStatusEnded, this);
}

void View::PaintOverlaysAccelerated(void)
{
	ViewPrivate::OverlaySurfaces::const_iterator iter = d->mOverlaySurfaces.begin();    
    ViewPrivate::OverlaySurfaces::const_iterator end = d->mOverlaySurfaces.end();    
    for (; iter < end; ++iter) 
    {
        EA::WebKit::FloatRect eaRect(iter->mRect.x(), iter->mRect.y(), iter->mRect.width(), iter->mRect.height());
        EA::WebKit::TransformationMatrix identity;
        d->mHardwareRenderer->RenderSurface(iter->mpSurface, eaRect, identity, 1.0f, NULL);
    }
}

void View::PaintOverlays(void)
{
    int overlayCount = d->mOverlaySurfaces.size();
    if (!overlayCount || (d->mDirtyRegions.empty()))
    {
        return;
    }

    ISurface::SurfaceDescriptor surfaceDescriptor = {0};
    d->mDisplaySurface->Lock(&surfaceDescriptor);    

    int surfaceW = 0;
    int surfaceH = 0;
    d->mDisplaySurface->GetContentDimensions(&surfaceW, &surfaceH);

    RefPtr<cairo_surface_t> mainCairoSurface = adoptRef(cairo_image_surface_create_for_data((unsigned char*)surfaceDescriptor.mData, CAIRO_FORMAT_ARGB32, surfaceW, surfaceH, surfaceDescriptor.mStride));    
	RefPtr<cairo_t> cairoContext = adoptRef(cairo_create(mainCairoSurface.get()));
	WebCore::GraphicsContext graphicsContext(cairoContext.get());
 
	ViewPrivate::OverlaySurfaces::const_iterator iter = d->mOverlaySurfaces.begin();    
    ViewPrivate::OverlaySurfaces::const_iterator end = d->mOverlaySurfaces.end();    
    for (; iter < end; ++iter) 
	{
        for (unsigned i = 0; i < d->mDirtyRegions.size(); ++i) 
		{
            if (iter->mRect.intersects(d->mDirtyRegions[i]))
            {
                // Just blit over the full surface so we don't have to do this multiple times if intersecting with many dirty rects 
                ISurface::SurfaceDescriptor overlayDescriptor = {0};
                ISurface* pOverlaySurface = iter->mpSurface;
                
                pOverlaySurface->Lock(&overlayDescriptor);    
				cairo_save(cairoContext.get());  
                
                int overlayW = 0;
                int overlayH = 0;
                pOverlaySurface->GetContentDimensions(&overlayW, &overlayH);

                RefPtr<cairo_surface_t> overlayCairoSurface = adoptRef(cairo_image_surface_create_for_data((unsigned char*)overlayDescriptor.mData, CAIRO_FORMAT_ARGB32, overlayW, overlayH, overlayDescriptor.mStride));    
				cairo_set_source_surface(cairoContext.get(), overlayCairoSurface.get(), iter->mRect.x(), iter->mRect.y());
				cairo_paint(cairoContext.get());    
                
				cairo_restore(cairoContext.get());
                pOverlaySurface->Unlock();  
                break;
            }
        }
    }

	cairo_surface_flush(mainCairoSurface.get());
    d->mDisplaySurface->Unlock();
}

View::View(void)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	
	d = new ViewPrivate(this);
}

View::~View()
{

	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	
	ShutdownView();

	
    delete d->mCachedCallstackNameArray;
    delete d->mCachedCallstackArgArray;
    delete d->mCachedCallstackLineArray;
    delete d->mCachedCallstackUrlArray;

	delete d;
	d = NULL;
}

bool View::InitView(const ViewParameters& vp)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	EAW_ASSERT_MSG(!d->mInitialized, "View has already been initialized.");

	if(!d->mInitialized)
	{
		d->mBackgroundColor = vp.mBackgroundColor;
		d->mpUserData		= vp.mpUserData;
		d->mUsingTiledBackingStore = (vp.mUseTiledBackingStore && vp.mHardwareRenderer); // Tiled backing storage can be used only with hardware renderer.
		
		ISurface *displaySurface = vp.mDisplaySurface;
		if(d->mUsingTiledBackingStore)
		{
			EAW_ASSERT_MSG(!displaySurface, "Don't pass a display surface if using tiled backing storage. Backing tiles will be created by EAWebKit.");
			d->mTileSize = vp.mTileSize;
			d->mRemoveNonVisibleTiles = vp.mRemoveNonVisibleTiles;
		}
		if(!displaySurface)
		{
			if(vp.mHardwareRenderer)
			{
				// If not using tiled backing store, we need to create a surface here for backing the view.
				// If using the tiled backing store, the backing tiles are part of the tiled backing store implementation and we don't need to create a 
				// the surface here.
				if(!d->mUsingTiledBackingStore)  
				{
					displaySurface = vp.mHardwareRenderer->CreateSurface();
				}
			}
			else
			{
				displaySurface = new SoftwareSurface();
			}
		}
		EAW_ASSERT_MSG((displaySurface || (vp.mHardwareRenderer && vp.mUseTiledBackingStore)), "No backing storage for the view");
		d->mDisplaySurface	= displaySurface;
		d->mHardwareRenderer = vp.mHardwareRenderer;

		EAW_ASSERT_MSG(!d->page, "Page for this view already exists !");
		d->page = new WebPage(this);

#if ENABLE(TILED_BACKING_STORE)
		if(d->mUsingTiledBackingStore)
		{
			Page()->handle()->page->settings()->setTiledBackingStoreEnabled(true);
			Page()->handle()->page->mainFrame()->tiledBackingStore()->SetUseTimers(false);
			Page()->handle()->page->mainFrame()->tiledBackingStore()->setTileSize(WebCore::IntSize(d->mTileSize,d->mTileSize));
		}
		else
		{
			Page()->handle()->page->settings()->setTiledBackingStoreEnabled(false);
		}
#endif
		SetSize(IntSize(vp.mWidth, vp.mHeight));
	}

	d->mInitialized = true;
	
	return true;
}

void View::ShutdownView(void)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	if(d->mInitialized)
	{
		WebCore::ResourceHandleManager::sharedInstance()->RemoveDependentJobs(this);

		if (d->page) 
		{
			if(d->page->GetInspector())
				d->page->GetInspector()->Shutdown();
			if(d->mUsingTiledBackingStore)
				d->page->handle()->page->settings()->setTiledBackingStoreEnabled(false);
		}

		if(d->mDisplaySurface)
		{
			d->mDisplaySurface->SetContentDimensions(0, 0);
			d->mDisplaySurface->Release();
			d->mDisplaySurface = NULL;
		}
		delete d->page;
		d->page = NULL;

		d->OverlayChangeNotify();
		EAW_ASSERT(!d->mOverlaySurfaces.size());
	}
	d->mInitialized = false;
}

bool View::IsUsingTiledBackingStore() const
{
	return d->mUsingTiledBackingStore;
}

WebPage *View::Page(void) const
{
	if (!d->page) 
    {
		View *that = const_cast<View*>(this);
		that->SetPage(new WebPage(that));
	}

	return d->page;
}

void View::SetPage(WebPage *page)
{
	if (d->page == page)
    {
		return;
    }

	d->page = page;

	if (d->page) 
    {
		d->page->setView(this);
	}
}

void View::SetURI(const char8_t *pUrl)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	if(pUrl && pUrl[0])
	{
		WTF::String url16 = WTF::String::fromUTF8(pUrl,strlen(pUrl));
		SetURI(url16.charactersWithNullTermination());
	}
}

void View::SetURI(const char16_t* pUrl)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	if(pUrl && pUrl[0])
	{
		d->OverlayChangeNotify();
		WebCore::KURL kurl(WebCore::KURL(), pUrl);
		Page()->mainFrame()->load(kurl);
	}
}

void View::SetHtml(const char* html, size_t length, const char* baseUrl /* = NULL */)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	WebCore::KURL kbaseUrl(WebCore::KURL(), baseUrl); 
	Page()->mainFrame()->setHtml(html, length, kbaseUrl);
}

void View::SetContent(const char8_t* data, size_t length, const char8_t* mediaType, const char8_t* baseUrl /* = NULL */)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	WebCore::KURL kbaseUrl(WebCore::KURL(), baseUrl);
	Page()->mainFrame()->setContent(data, length, mediaType, kbaseUrl);
}

const char16_t* View::Title(void) const
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	if (d->page)
	{
		WTF::String titleStr = d->page->mainFrame()->title();
		d->mTitle.assign(titleStr.characters(), titleStr.length());
		return d->mTitle.c_str();
	}

    return NULL;
}


const char16_t* View::GetURI(void) const
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	
	if (d->page)
	{
		WTF::String urlStr = d->page->mainFrame()->url().string();
		d->mURL.assign(urlStr.characters(), urlStr.length());
		return d->mURL.c_str();
	}

    return NULL;
}

const char16_t *View::GetEncodedURI(const char16_t *url)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
    
    if (url == NULL) 
    {
        return NULL;
    }

    WebCore::KURL kurl(WebCore::KURL(), url);
    d->mEncodedURICache.assign(kurl.string().characters(), kurl.string().length());

    return d->mEncodedURICache.c_str();
}

const char16_t *View::GetEncodedURI(const char8_t *url)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);

    if (url == NULL)
    {
        return NULL;
    }

    WTF::String url16 = WTF::String::fromUTF8(url, strlen(url));
    return GetEncodedURI(url16.charactersWithNullTermination());
}

void View::triggerPageAction(WebAction action)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	Page()->triggerAction(action);
}

void View::CancelLoad(void)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	if (d->page)
    {
		d->page->triggerAction(EA::WebKit::Stop);
    }
}

void View::GoBack(void)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	SET_AUTO_COLLECTOR_STACK_BASE();   

    d->OverlayChangeNotify();

    if (d->page)
    {
        d->page->triggerAction(EA::WebKit::Back);
    }
}

void View::GoForward(void)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	SET_AUTO_COLLECTOR_STACK_BASE();   

    d->OverlayChangeNotify();

    if (d->page) 
    {
		d->page->triggerAction(EA::WebKit::Forward);
    }
}

void View::Refresh(void)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	
    d->OverlayChangeNotify();

    if (d->page)
    {
		d->page->triggerAction(EA::WebKit::Reload);
    }
}

void View::SetSize(IntSize size)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
    
    d->OverlayChangeNotify();

	if (d->page)
    {
		d->page->setViewportSize(size);
    }

	EAW_ASSERT_MSG(d->mDisplaySurface || d->mUsingTiledBackingStore, "Main display Surface should be allocated before setting the dimensions if not using tiled backing storage");
	if (d->mDisplaySurface) 
    {
        d->mDisplaySurface->SetContentDimensions(size.mWidth, size.mHeight);
    	
		// Clear any old dirty regions since the resize could have invalidated them.
        d->mDirtyRegions.clear();
    }
	
	InvalidateFullView();
}

IntSize View::GetSize(void) const
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	if(d->page)
    {
		return d->page->viewportSize();
    }

	return IntSize(0, 0);
}

void View::SaveSurfacePNGHelper(const char8_t *filepath, EA::WebKit::ISurface* surface)
{
	int width = 0;
	int height = 0;
	surface->GetContentDimensions(&width, &height);

	ISurface::SurfaceDescriptor desc = {0};
	surface->Lock(&desc);

	cairo_surface_t* cairoSurface = cairo_image_surface_create_for_data((unsigned char*)desc.mData, CAIRO_FORMAT_ARGB32, width, height, desc.mStride);    
	cairo_surface_write_to_png(cairoSurface, filepath);
	cairo_surface_destroy(cairoSurface);

	surface->Unlock();
}
void View::SaveSurfacePNG(const char8_t *filepath)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);

	if(!filepath || !filepath[0])
		return;

	if(!HardwareAccelerated())
	{
		WebCore::makeAllDirectories(WebCore::directoryName(filepath));
		SaveSurfacePNGHelper(filepath,d->mDisplaySurface);
	}
	else
	{
		// Turn the file path into a directory since we are going to save multiple surfaces. We can add
		// another API but this would work fine since it is a debug API.
		eastl::string8 sPath(filepath);
		sPath.erase(sPath.find(".png"));
#if ENABLE(TILED_BACKING_STORE)
		if(IsUsingTiledBackingStore())
		{
			int tileIndex = 0;
			eastl::string8 tilesDir(eastl::string8::CtorSprintf(),"%s/tiles",sPath.c_str());
			WebCore::makeAllDirectories(tilesDir.c_str());

			ViewPrivate::Tiles::const_iterator it = d->mTiles.begin();
			ViewPrivate::Tiles::const_iterator itEnd = d->mTiles.end();
			for(; it != itEnd;++it)
			{
				if(EA::WebKit::ISurface* tileSurface = (*it)->GetSurface())
				{
					eastl::string8 tilePath(eastl::string8::CtorSprintf(),"%s/tile_%d.png",tilesDir.c_str(),tileIndex++);
					SaveSurfacePNGHelper(tilePath.c_str(),tileSurface);			
				}
			}
		}
		else
		{
			eastl::string8 surfacePath = sPath;
			WebCore::makeAllDirectories(sPath.c_str());
			
			surfacePath.append("/MainBackingSurface.png");
			SaveSurfacePNGHelper(surfacePath.c_str(),d->mDisplaySurface);	
		}
#endif

#if USE(ACCELERATED_COMPOSITING)
		int textureIndex = 0;
		eastl::string8 texturesDir(eastl::string8::CtorSprintf(),"%s/textures",sPath.c_str());
		WebCore::makeAllDirectories(texturesDir.c_str());
		
		ViewPrivate::Textures::const_iterator it = d->mTextures.begin();
		ViewPrivate::Textures::const_iterator itEnd = d->mTextures.end();
		for(; it != itEnd;++it)
		{
			if(EA::WebKit::ISurface* textureSurface = (*it)->GetSurface())
			{
				eastl::string8 texturePath(eastl::string8::CtorSprintf(),"%s/texture_%d.png",texturesDir.c_str(),textureIndex++);
				SaveSurfacePNGHelper(texturePath.c_str(),textureSurface);			
			}
		}
#endif
	}


}

void View::OnMouseMoveEvent(const EA::WebKit::MouseMoveEvent& ev)
{
    if (d->mBeingDebugged)
    {
        return;
    }

	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	SET_AUTO_COLLECTOR_STACK_BASE();  

	d->mCursorPos.mX = ev.mX;
	d->mCursorPos.mY = ev.mY;

    // We need to inject the mouse button id into the move event so we can detected mouse drag events.
    EA::WebKit::MouseMoveEvent mouseEv(ev);
    mouseEv.mId = d->mMouseButtonPressedId;

    // Question: Should the overlay client really be intercepting the move event no matter what? What about when a tooltip is up?
    // Maybe we should only intercept if the mouse event is handled.
	// Answer: Yes, we need to fix this. 
    if (d->mpOverlayInputClient)
    {
        d->mpOverlayInputClient->OnMouseMoveEvent(mouseEv);
    }
	else if (d->page) 
    {
        d->page->handle()->mouseMoveEvent(mouseEv);
    }
}

void View::OnMouseButtonEvent(const EA::WebKit::MouseButtonEvent&  ev)
{
    if (d->mBeingDebugged)
    {
        return;
    }

	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	SET_AUTO_COLLECTOR_STACK_BASE();  

    if (d->mpOverlayInputClient)
    {
        if(ev.mbDepressed)
            d->mMouseButtonPressedId = ev.mId;          
		else
            d->mMouseButtonPressedId = kMouseNoButton;  // Reset
        
        d->mpOverlayInputClient->OnMouseButtonEvent(ev);
    }
	else if (d->page) 
	{
        if(ev.mbDepressed)
        {
            d->mMouseButtonPressedId = ev.mId;          
			d->page->handle()->mousePressEvent(ev);
        }
		else
        {
            d->mMouseButtonPressedId = kMouseNoButton;  // Reset
			d->page->handle()->mouseReleaseEvent(ev);
        }
    }
}

void View::OnMouseWheelEvent(const EA::WebKit::MouseWheelEvent& ev)
{
    if (d->mBeingDebugged)
    {
        return;
    }

	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	SET_AUTO_COLLECTOR_STACK_BASE();  

    if (d->mpOverlayInputClient)
    {
        d->mpOverlayInputClient->OnMouseWheelEvent(ev);
    }
	else if (d->page)
    {
        d->page->handle()->wheelEvent(ev);
    }
}

void View::OnKeyboardEvent(const EA::WebKit::KeyboardEvent& ev)
{
    if (d->mBeingDebugged)
    {
        return;
    }

	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	SET_AUTO_COLLECTOR_STACK_BASE();  

    if (d->mpOverlayInputClient)
    {
        d->mpOverlayInputClient->OnKeyboardEvent(ev);
    }
	else if (d->page)
	{
        if(ev.mbDepressed)
        {
			d->page->handle()->keyPressEvent(ev);
        }
		else
        {
			d->page->handle()->keyReleaseEvent(ev);
        }
	}
}

void View::OnFocusChangeEvent(bool hasFocus)
{
    if (d->mBeingDebugged)
    {
        return;
    }

	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
    
    d->mMouseButtonPressedId = kMouseNoButton;  // Reset. 

    if (d->page)
	{
		if(hasFocus)
        {
			d->page->handle()->focusInEvent();
        }
		else
        {
			d->page->handle()->focusOutEvent();
        }
	}
}

void View::SetTextIntoSelectedInput(const char16_t* text, bool textWasAccepted)
{
    if (d->mBeingDebugged)
    {
        return;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);   

	WebCore::Frame *pFrame = GetFrame();
		
	while(pFrame)
	{
		WebCore::Document* document = pFrame->document();
		EAW_ASSERT(document);

		WebCore::Node* node = document->focusedNode();
		if (node && node->isHTMLElement())
		{
			if(node->hasTagName(WebCore::HTMLNames::inputTag)) 
			{
				WebCore::HTMLInputElement* pInputElement = static_cast<WebCore::HTMLInputElement*> (node);

				if(textWasAccepted && pInputElement->isTextField())
				{
					pInputElement->setValue(text);                   
				}
				pInputElement->blur();//We want to blur even if the text was not accepted and only if the element was an editable field.
			}
			else if(node->hasTagName(WebCore::HTMLNames::textareaTag))
			{
				WebCore::HTMLTextAreaElement* pTextAreaElement = static_cast<WebCore::HTMLTextAreaElement*> (node);

				if(textWasAccepted)
				{
					pTextAreaElement->setValue(text);                   
				}
				pTextAreaElement->blur();//We want to blur even if the text was not accepted and only if the element was an editable field.
			}

			break;
		}		

		pFrame = pFrame->tree()->traverseNext();
	}
}



WTF::String GetTextFromSelectedInputAsString(EA::WebKit::View* view, bool contentText)
{
	WebCore::Frame *pFrame = view->GetFrame();
	
	while(pFrame)
	{
		WebCore::Document* document = pFrame->document();
		EAW_ASSERT(document);

		if(document)
		{
			WebCore::Node* node = document->focusedNode();
			if( node && node->isHTMLElement()) 
			{
				if(node->hasTagName( WebCore::HTMLNames::inputTag)) 
				{
					WebCore::HTMLInputElement* pInputElement = static_cast<WebCore::HTMLInputElement*> (node);
					if(pInputElement->isTextField())
					{
						if(contentText)
						{
							return pInputElement->value();                
						}
						else
						{
							return pInputElement->getAttribute("title").string();
						}
					}
				}
				else if(node->hasTagName(WebCore::HTMLNames::textareaTag))
				{
					WebCore::HTMLTextAreaElement* pTextAreaElement = static_cast<WebCore::HTMLTextAreaElement*> (node);
					if(contentText)
					{
						return pTextAreaElement->value();
					}
					else
					{
						return pTextAreaElement->getAttribute("title").string();
					}
				}

				break;
			}
		}


		pFrame = pFrame->tree()->traverseNext();
	}

	return WTF::String();
}


uint32_t View::GetTextFromSelectedInput(char16_t* contentTextBuffer, const uint32_t maxcontentTextBufferLength, char16_t* titleTextBuffer, const uint32_t maxTitleTextBufferLength)
{
	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);   

	uint32_t size = 0;
	if(titleTextBuffer && maxTitleTextBufferLength)
	{
		WTF::String val = GetTextFromSelectedInputAsString(this,false);
		if(!val.isEmpty())                
		{
			size = val.length() + 1;    // + 1 For terminator 
			const UChar* pSource = val.charactersWithNullTermination();
			EAW_ASSERT_MSG(size <=maxTitleTextBufferLength, "Truncating the string as buffer length is not sufficient\n");
			EA::Internal::Strlcpy(titleTextBuffer, pSource, maxTitleTextBufferLength);
			titleTextBuffer[maxTitleTextBufferLength-1] = 0;
		}
		else
		{
			titleTextBuffer[0] = 0;
		}
	}

	size = 0;
	if(contentTextBuffer && maxcontentTextBufferLength)
	{
		WTF::String val = GetTextFromSelectedInputAsString(this,true);
		if(!val.isEmpty())                
		{
			size = val.length() + 1; 
			const UChar* pSource = val.charactersWithNullTermination();
			EAW_ASSERT_MSG(size <=maxcontentTextBufferLength, "Truncating the string as buffer length is not sufficient\n");
			EA::Internal::Strlcpy(contentTextBuffer, pSource, maxcontentTextBufferLength);
			contentTextBuffer[maxcontentTextBufferLength-1] = 0;
		}
		else
		{
			contentTextBuffer[0] = 0;
		}
	}

	//Returning size is not so useful as we don't allow embedded NULL and now we have two buffers passed in. For now, we return the size of the content to be backward compatible.
	return size;    
}


bool View::focusNextPrevChild(bool next)
{
    if (d->mBeingDebugged)
    {
        return false;
    }

	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
#if 0
	if (d->page && d->page->focusNextPrevChild(next))
		return true;
#endif	
	return false;
}

void View::ShowInspector(bool state)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	Page()->SetInspectorDisplay(state);
}

void View::SetZoomFactor(float factor)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
    if (d->page)  
    { 
        WebFrame *frame = d->page->mainFrame();
        if (frame)
        {
            frame->setZoomFactor(factor);       
            d->OverlayChangeNotify();
        }
    }
}

float View::ZoomFactor(void) const
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
    float factor = 1.0f;

    if (d->page)  
    { 
        WebFrame *frame = d->page->mainFrame();
        if (frame)
            factor = frame->zoomFactor();       
    }
    return factor;
}

void View::SetDrawDebugVisuals(bool on)
{
	d->mDrawDebugVisuals = on;
	InvalidateFullView();
}

bool View::ShouldDrawDebugVisuals() const
{
	return d->mDrawDebugVisuals;
}

void View::InvalidateFullView()
{
	if(d->mDisplaySurface)
	{
		IntRect fullSurfaceRect(IntPoint(0,0), GetSize());
		AddDirtyRegion(fullSurfaceRect);
	}

#if ENABLE(TILED_BACKING_STORE)
	if (d->mUsingTiledBackingStore)
	{
		if (WebCore::TiledBackingStore *backingStore = WebFramePrivate::core(d->page->mainFrame())->tiledBackingStore())
		{
			// Force the entire backing store to update by using a very large size.
			const IntRect largeRect(IntPoint(0,0), IntSize(100000,100000));           
			backingStore->invalidate(largeRect);
		}
	}
#endif
}
// Constructors for the metrics callback system
ViewProcessInfo::ViewProcessInfo(void)
: mpView(0),
mProcessType(kVProcessTypeNone),
mProcessStatus(kVProcessStatusNone),
mStartTime(0.0),
mIntermediateTime(0.0),
mURI(0),
mSize(0),
mJobId(0)
{
    // Do nothing.
}

ViewProcessInfo::ViewProcessInfo(VProcessType type,VProcessStatus status, View* pView)
: mpView(pView),
mProcessType(type),
mProcessStatus(status),
mStartTime(0.0),
mIntermediateTime(0.0),
mURI(0),
mSize(0),
mJobId(0)
{
	// Do nothing.
}

void ViewProcessInfo::ResetTime(void)
{
	mStartTime = 0.0;
	mIntermediateTime = 0.0;
}

//Global array for profile process.
ViewProcessInfo gProcessInfoArray[kVProcessTypeLast];

void NOTIFY_PROCESS_STATUS(VProcessType processType, VProcessStatus processStatus, EA::WebKit::View *pView)
{
 	const Parameters& parameters = EA::WebKit::GetParameters(); 
 	if(!parameters.mEnableProfiling)
 		return;

	EAW_ASSERT_MSG(processType<kVProcessTypeLast, "Size mismatch. The process enum is higher than the size of the array.");
	if(processType<kVProcessTypeLast)
	{
		gProcessInfoArray[processType].mpView = pView;
		if(pView)
			gProcessInfoArray[processType].mpUserData = pView->GetUserData();

		gProcessInfoArray[processType].mProcessType = processType; //lame but works(since we don't initialize the global array with each process type.
		gProcessInfoArray[processType].mProcessStatus = processStatus;

		if(processStatus == kVProcessStatusStarted)
			gProcessInfoArray[processType].ResetTime();

		EA::WebKit::EAWebKitClient* const pViewNotification = EA::WebKit::GetEAWebKitClient();
		if(pViewNotification)
			pViewNotification->ViewProcessStatus(gProcessInfoArray[processType]);

	}
}

void NOTIFY_PROCESS_STATUS(ViewProcessInfo& process, VProcessStatus processStatus)
{
 	const Parameters& parameters = EA::WebKit::GetParameters(); 
 	if(!parameters.mEnableProfiling)
 		return;

	if(processStatus == kVProcessStatusStarted)
		process.ResetTime();

	process.mProcessStatus = processStatus;

	EA::WebKit::EAWebKitClient* const pViewNotification = EA::WebKit::GetEAWebKitClient();
	if(pViewNotification)
		pViewNotification->ViewProcessStatus(process);
}

void View::Tick()
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	SET_AUTO_COLLECTOR_STACK_BASE();  

	NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeViewTick, EA::WebKit::kVProcessStatusStarted, this);
	WTF::dispatchFunctionsFromMainThread(); // This is required for any children threads to work, for example, web workers.    
	
	WebCore::ResourceHandleManager::sharedInstance()->TickDownload();
	NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeTransportTick, EA::WebKit::kVProcessStatusStarted, this);
	WebCore::ResourceHandleManager::sharedInstance()->TickTransportHandlers();
	if(SocketTransportHandler* pSocketTxHandler = EA::WebKit::GetSocketTransportHandler())
		pSocketTxHandler->Tick();
	NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeTransportTick, EA::WebKit::kVProcessStatusEnded, this);

	WebCore::fireTimerIfNeeded();

    Paint();
    NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeViewTick, EA::WebKit::kVProcessStatusEnded, this);
}

bool View::EvaluateJavaScript(const char16_t *source, JavascriptValue *resultOut)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	SET_AUTO_COLLECTOR_STACK_BASE();   // Need to store collector stack base as this call can bypass the viewTick()

    if (d->page && d->page->mainFrame())
    {
        WTF::String scriptSource((UChar*)source);
        return d->page->mainFrame()->evaluateJavaScript(scriptSource, resultOut);
    }

    return false;
}

bool View::EvaluateJavaScript(const char8_t *source, JavascriptValue *resultOut)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	SET_AUTO_COLLECTOR_STACK_BASE();   // Need to store collector stack base as this call can bypass the viewTick()

    if (d->page && d->page->mainFrame())
    {
        WTF::String scriptSource(source);
        return d->page->mainFrame()->evaluateJavaScript(scriptSource, resultOut);
    }

    return false;
}

void View::BindJavaScriptObject(const char8_t *name, IJSBoundObject *obj)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	
	if (d->page && d->page->mainFrame())
    {
        d->mBoundObjects[name] = obj;
        d->page->mainFrame()->addToJavaScriptWindowObject(name, obj);
    }
}

void View::RebindJSBoundObjects(void)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);   
	
    for (ViewPrivate::JSObjectMap::iterator iter = d->mBoundObjects.begin(), end = d->mBoundObjects.end(); iter != end; ++iter)
    {
        d->page->mainFrame()->addToJavaScriptWindowObject(iter->first.c_str(), iter->second);
    }
}

void View::GetCallstack(ReportJSCallstackInfo *callStackOut)
{
    SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);

    // Fix contributed by YC Chin: Need to clear the old callstack before querying, so subsequent 
    // calls won't keep pushing to the back without removing the previous items.
    d->mLastCallstackNames.clear();
    d->mLastCallstackArguments.clear();
    d->mLastCallstackLines.clear();
    d->mLastCallstackUrls.clear();

    d->page->mainFrame()->GetJavaScriptCallstack(&d->mLastCallstackNames, &d->mLastCallstackArguments, &d->mLastCallstackLines, &d->mLastCallstackUrls);

    delete d->mCachedCallstackNameArray;
    delete d->mCachedCallstackArgArray;
    delete d->mCachedCallstackLineArray;
    delete d->mCachedCallstackUrlArray;

    d->mCachedCallstackNameArray = new const char*[d->mLastCallstackNames.size()];
    d->mCachedCallstackArgArray = new const char*[d->mLastCallstackArguments.size()];
    d->mCachedCallstackLineArray = new int[d->mLastCallstackLines.size()];
    d->mCachedCallstackUrlArray = new const char*[d->mLastCallstackUrls.size()];

    EAW_ASSERT(d->mLastCallstackNames.size() == d->mLastCallstackArguments.size() 
               && d->mLastCallstackNames.size() == d->mLastCallstackLines.size() 
               && d->mLastCallstackNames.size() == d->mLastCallstackUrls.size());

    for (int i = 0; i < (int) d->mLastCallstackNames.size(); ++i)
    {
        d->mCachedCallstackNameArray[i] = d->mLastCallstackNames[i].c_str();
        d->mCachedCallstackArgArray[i] = d->mLastCallstackArguments[i].c_str();
        d->mCachedCallstackLineArray[i] = d->mLastCallstackLines[i];
        d->mCachedCallstackUrlArray[i] = d->mLastCallstackUrls[i].c_str();
    }

    callStackOut->mDepth = d->mLastCallstackNames.size();
    callStackOut->mNames = d->mCachedCallstackNameArray;
    callStackOut->mArgs = d->mCachedCallstackArgArray;
    callStackOut->mLines = d->mCachedCallstackLineArray;
    callStackOut->mUrls = d->mCachedCallstackUrlArray;
}

void *View::GetJSExecState(void) 
{
    return d->page->mainFrame()->GetExec();
}

void* View::GetUserData(void) const
{
	return d->mpUserData;
}

uint32_t View::GetBackgroundColor(void) const
{
    return d->mBackgroundColor;
}

void View::AddDirtyRegion(const IntRect &region) 
{
    WebCore::IntRect wcRegion(region);
	ViewPrivate::DirtyRegions& dirtyRegions = d->mDirtyRegions;
    for (uint32_t iRegion = 0; iRegion < dirtyRegions.size(); ++iRegion) 
	{
        WebCore::IntRect &iRegionRect = dirtyRegions[iRegion];
        if (iRegionRect.intersects(wcRegion)) 
		{
            // If the region is fully contained there's no need to merge later.
            if (iRegionRect.contains(wcRegion)) 
			{
                return;
            }

            // Otherwise, unite the rects, and execute the merge code.
            iRegionRect.unite(wcRegion);
            
            // Repeatedly merge the rects until there is no change.
            // This uses a goto because it's the easiest way to break out of
            // a double nested loop and restart the process.
RestartMerge:
            const uint32_t dirtyRegionCount = dirtyRegions.size();
            for (uint32_t i = 0; i < dirtyRegionCount; ++i) 
			{
                WebCore::IntRect &iRect = dirtyRegions[i];
                for (uint32_t j = i + 1; j < dirtyRegionCount; ++j) 
				{
                    const WebCore::IntRect &jRect = dirtyRegions[j];
                    if (iRect.intersects(jRect)) 
					{
                        iRect.unite(jRect);
                        dirtyRegions.erase(dirtyRegions.begin() + j);
                        goto RestartMerge;
                    }
                }
            }

#if _DEBUG
			// There should be no intersecting rects in dirtyRegions at this point.
            for (uint32_t i = 0; i < dirtyRegions.size(); ++i) 
			{
                for (uint32_t j = i+1; j < dirtyRegions.size(); ++j) 
				  {
                    EAW_ASSERT_MSG(dirtyRegions[i].intersects(dirtyRegions[j]) == false, "Intersecting Dirty rects");
                }
            }
#endif
            return;
        }
    }

    // The region didn't intersect with anything, add it as a new region.
    dirtyRegions.push_back(wcRegion);
}

ISurface* View::GetDisplaySurface(void) 
{
	return d->mDisplaySurface;
}

IHardwareRenderer* View::GetHardwareRenderer(void) 
{
    return d->mHardwareRenderer;
}

bool View::ShouldRemoveNonVisibleTiles() const
{
	return d->mRemoveNonVisibleTiles;
}
void View::AddTile(WebCore::TileEA* tile)
{
#if ENABLE(TILED_BACKING_STORE)
	d->mTiles.push_back(tile);
#endif
}
void View::RemoveTile(WebCore::TileEA* tile)
{
#if ENABLE(TILED_BACKING_STORE)
	d->mTiles.remove(tile);
#endif
}

void View::AddTexture(WebCore::BitmapTextureEA* texture)
{
#if USE(ACCELERATED_COMPOSITING)
	d->mTextures.push_back(texture);
#endif
}
void View::RemoveTexture(WebCore::BitmapTextureEA* texture)
{
#if USE(ACCELERATED_COMPOSITING)
	d->mTextures.remove(texture);
#endif
}


ISurface* View::CreateOverlaySurface(int x, int y, int width, int height)
{
    ISurface* pSurface = NULL;
    if (HardwareAccelerated())
	{
        pSurface = d->mHardwareRenderer->CreateSurface();
    }
    else
    {
        pSurface = new SoftwareSurface();
    }
    
    pSurface->SetContentDimensions(width, height);

    OverlaySurfaceInfo info(pSurface, x, y, width, height);
    d->mOverlaySurfaces.push_back(info);
    
    return pSurface;
}

void View::DestroyOverlaySurface(ISurface* pOverlaySurface)
{
    EAW_ASSERT(pOverlaySurface);
    ViewPrivate::OverlaySurfaces::iterator iter = d->mOverlaySurfaces.begin();    
    ViewPrivate::OverlaySurfaces::const_iterator end = d->mOverlaySurfaces.end();    
    for (; iter < end; ++iter) 
	{
        if (iter->mpSurface == pOverlaySurface)
        {
            AddDirtyRegion(iter->mRect); // Request a redraw to clean up area under the overlay surface. 

            iter->mpSurface->SetContentDimensions(0, 0);
            iter->mpSurface->Release();
            iter->mpSurface = NULL;
            d->mOverlaySurfaces.erase(iter);
            break;
        }
    }
}

void View::SetOverlayInputClient(IOverlayInputClient* pClient)
{
    d->mpOverlayInputClient = pClient;
}

IOverlayInputClient* View::GetOverlayInputClient() const
{
	return d->mpOverlayInputClient;
}

void View::SetJumpNavigationParams(const JumpNavigationParams& jumpNavigationParams)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	d->mNavDelegate.SetJumpNavigationParams(jumpNavigationParams);
}

bool View::JumpToNearestElement(EA::WebKit::JumpDirection direction)
{
    if (d->mBeingDebugged)
    {
        return false;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);   
	return d->mNavDelegate.JumpToNearestElement(direction);
}

bool View::JumpToId(const char* jumpToId)
{
    if (d->mBeingDebugged)
    {
        return false;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);
	return d->mNavDelegate.JumpToId(jumpToId);
}

bool View::ClickElementById(const char* id)
{
    if (d->mBeingDebugged)
    {
        return false;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);   
	return d->mNavDelegate.ClickElementById(id);
}

bool View::ClickElementsByClass(const char* className)
{
    if (d->mBeingDebugged)
    {
        return false;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);   
	return d->mNavDelegate.ClickElementsByClass(className);
}

bool View::ClickElementsByIdOrClass(const char* idOrClassName)
{
    if (d->mBeingDebugged)
    {
        return false;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended); 
	return d->mNavDelegate.ClickElementsByIdOrClass(idOrClassName);
}	

bool View::Click()
{
    if (d->mBeingDebugged)
    {
        return false;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);   
	return d->mNavDelegate.Click();
}

void View::MoveMouseCursorToNode(WebCore::Node* node, bool scrollIfNecessary)
{
	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);
	d->mNavDelegate.MoveMouseCursorToNode(node, scrollIfNecessary);
}

void View::UpdateCachedHints(WebCore::Node* node)
{
	SET_AUTOFPUPRECISION(kFPUPrecisionExtended); 
	d->mNavDelegate.UpdateCachedHints(node);
}

void View::GetCursorPosition(int& x, int& y) const
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	
	x = d->mCursorPos.mX;
	y = d->mCursorPos.mY;
}

void View::SetCursorPosition(int x, int y)
{
    if (d->mBeingDebugged)
    {
        return;
    }

	SET_AUTOFPUPRECISION(kFPUPrecisionExtended);   
	{
		int lastX		= 0;
		int lastY		= 0;
		this->GetCursorPosition( lastX, lastY );

		int newX		= x;
		int newY		= y;

		EA::WebKit::MouseMoveEvent moveEvent;
		memset( &moveEvent, 0, sizeof(moveEvent) );

		int width  = GetSize().mWidth;
		int height = GetSize().mHeight;

		moveEvent.mX	= Clamp(0, newX, width);
		moveEvent.mY	= Clamp(0, newY, height);
		this->OnMouseMoveEvent( moveEvent );
	}
}

bool View::IsEmulatingConsoleOnPC() const
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return d->mEmulatingConsoleOnPC;
}

void View::SetEmulatingConsoleOnPC(bool emulatingConsoleOnPC)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	d->mEmulatingConsoleOnPC = emulatingConsoleOnPC;
}

WebCore::Frame* View::GetFrame(bool focusFrame /* = false */) const
{
	WebFrame *webFrame = focusFrame ? d->page->focusedFrame() : d->page->mainFrame();
	if(webFrame)
	{
		WebCore::Frame *pFrame = WebFramePrivate::core(webFrame);
		return pFrame;
	}
	return NULL;
}

View *CreateView(void) 
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	return new View();
}

void DestroyView(View* pView)
{
	SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
	delete pView;
}

void View::SetBeingDebugged(bool state)
{
    d->mBeingDebugged = state;
}

void View::SetToolTip(const char16_t* tip)
{
    d->mToolTip.show(tip, d->mCursorPos.mX, d->mCursorPos.mY);
}

}}
