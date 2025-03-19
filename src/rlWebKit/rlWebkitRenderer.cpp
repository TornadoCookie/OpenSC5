
#include "rlWebkitRenderer.h"

#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebkitAllocator.h>
#include <EAWebKit/EAWebKitFileSystem.h>
#include <EAWebKit/EAWebKitClient.h>
#include <EAWebKit/EAWebKitView.h>
#include "EAWebKit/EAWebKitTextInterface.h"

#include <vector>
#include <array>
#include <iostream>

RLRenderer::RLRenderer()
{

}

RLRenderer::~RLRenderer()
{

}

EA::WebKit::ISurface * RLRenderer::CreateSurface(void)
{
    RLSurface* res = new RLSurface();
    //if (data && length)
    //{
    //    EA::WebKit::ISurface::SurfaceDescriptor sd = {};
    //    res->Lock(&sd);
    //    memcpy(sd.mData, data, length);
    //    res->Unlock();
    //}
    return res;
}

void RLRenderer::SetRenderTarget(EA::WebKit::ISurface *target)
{
    std::cout << __FUNCTION__ << std::endl;
}

void RLRenderer::RenderSurface(EA::WebKit::ISurface *surface, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, float opacity, EA::WebKit::ISurface *mask)
{
   std::cout << __FUNCTION__ << std::endl;
}

//void RLRenderer::FillColor(uint32_t premultiplied_rgba32, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, EA::WebKit::CompositOperator op)
//{
//    std::cout << __FUNCTION__ << std::endl;
//}

//void RLRenderer::Clear(EA::WebKit::ClearFlags flags, uint32_t premultiplied_rgba32, float z, uint32_t stencil)
//{
//    std::cout << __FUNCTION__ << std::endl;
//}
void RLRenderer::BeginClip(EA::WebKit::TransformationMatrix &matrix, EA::WebKit::FloatRect &target)
{
    std::cout << __FUNCTION__ << std::endl;
}

void RLRenderer::EndClip(void)
{
    std::cout << __FUNCTION__ << std::endl;
}

void RLRenderer::BeginPainting(void)
{
   std::cout << __FUNCTION__ << std::endl;
}

void RLRenderer::EndPainting(void)
{
    std::cout << __FUNCTION__ << std::endl;
}



//------------------------GL Surface ------------------------------------


RLSurface::RLSurface()
{

}

RLSurface::~RLSurface()
{

}

void RLSurface::Lock(SurfaceDescriptor *pSDOut, const EA::WebKit::IntRect *rect /*= NULL*/)
{

}

void RLSurface::Unlock(void)
{

}

void RLSurface::Release(void)
{

}

bool RLSurface::IsAllocated(void) const
{
   return false;
}

void RLSurface::Reset(void)
{
    // no idea what this is supposed to do
}

void RLSurface::AllocateSurface(int width, int height)
{
 
}
