
#pragma once

#include <EAWebKit/EAWebKitSurface.h>
#include <EAWebKit/EAWebKitHardwareRenderer.h>

#include <cstdint>

class RLSurface : public EA::WebKit::ISurface
{
public:
   unsigned int tex = 0;
public:
   RLSurface();
   virtual ~RLSurface();
   virtual void Lock(SurfaceDescriptor *pSDOut, const EA::WebKit::IntRect *rect = NULL) override;
   virtual void Unlock(void) override;
   virtual void Release(void) override;
   virtual bool IsAllocated(void) const override;
   virtual void Reset(void) override;
protected:
   virtual void AllocateSurface(int width, int height) override;
};

class RLRenderer : public EA::WebKit::IHardwareRenderer
{
public:
   RLRenderer();
   virtual ~RLRenderer();
   virtual EA::WebKit::ISurface * CreateSurface(void) override;
   virtual void SetRenderTarget(EA::WebKit::ISurface *target) override;
   virtual void RenderSurface(EA::WebKit::ISurface *surface, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, float opacity, EA::WebKit::ISurface *mask) override;
   virtual void BeginClip(EA::WebKit::TransformationMatrix &matrix, EA::WebKit::FloatRect &target) override;
   virtual void EndClip(void) override;
   virtual void BeginPainting(void) override;
   virtual void EndPainting(void)  override;
};
