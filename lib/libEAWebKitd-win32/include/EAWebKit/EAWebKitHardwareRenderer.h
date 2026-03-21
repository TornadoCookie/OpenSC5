/*
Copyright (C) 2012 Electronic Arts, Inc.  All rights reserved.

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

#ifndef EAWEBKIT_HARDWARE_RENDERER_H
#define EAWEBKIT_HARDWARE_RENDERER_H

#include <EAWebKit/EAWebKitSystem.h>

namespace EA { namespace WebKit 
{
    class ISurface;

    // Implement this class and pass it through the view's initialization
    // parameters to enable hardware acceleration.
    // You will also need to implement a version of the ISurface class that can work with the 
    // renderer.

    class IHardwareRenderer
    {
    public:
        virtual ~IHardwareRenderer(void) {}

        // You should return a new instance of your Hardware ISurface implementation.
        // Note that there is no 'DestroySurface' call, so it's up to the ISurface to nuke itself
        // when ISurface::Release is called.
        virtual EA::WebKit::ISurface *CreateSurface(void) = 0;

        // Your hardware renderer needs to support rendering to your ISurface implementation returned from
        // CreateSurface call. 
        // If target is null, the target should be the screen. (This seems a bit iffy as it is not clear what screen means).
        virtual void SetRenderTarget(EA::WebKit::ISurface *target) = 0;

		// target specifies the part of the surface that needs to be renderer. 
		// matrix specifies the location and orientation of the surface to be rendered in the containing view coordinates. It is possible
		// that the location is beyond the viewport size in which case the surface is not visually rendered.
        // The masking surface is an alpha mask.
        virtual void RenderSurface(EA::WebKit::ISurface *surface, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, float opacity, EA::WebKit::ISurface *mask) = 0;
        
        // The renderer also needs to support clipping arbitrary regions. One option is to use a stencil buffer to do this.
        virtual void BeginClip(EA::WebKit::TransformationMatrix &matrix, EA::WebKit::FloatRect &target) = 0;
        virtual void EndClip(void) = 0;

        // These are here in case you need to be notified that the renderer is about to start or stop painting.
        virtual void BeginPainting(void) {}
        virtual void EndPainting(void) {}
    };
}}

#endif
